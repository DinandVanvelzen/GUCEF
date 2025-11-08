use std::io::{BufRead, BufReader};
use std::process::{Command, Stdio};
use std::thread;
use clap::Parser;
use k3s_discover::listener_lib::parse_avahi_line;

/// Simple Avahi-based DNS-SD listener for _k3s-discover._tcp
#[derive(Parser, Debug)]
#[command(author, version, about)]
struct Args {
    /// Automatically run join command when a candidate is discovered
    #[arg(long)]
    auto_join: bool,

    /// Path to join command/script
    #[arg(long, default_value = "/usr/local/bin/k3s-discover-join.sh")]
    join_cmd: String,
}

fn main() {
    let args = Args::parse();

    // Spawn avahi-browse in realtime/resolved mode
    let mut child = match Command::new("avahi-browse")
        .arg("-rt")
        .arg("_k3s-discover._tcp")
        .stdout(Stdio::piped())
        .spawn()
    {
        Ok(c) => c,
        Err(e) => {
            eprintln!("Failed to spawn avahi-browse: {}. Make sure Avahi is installed.", e);
            return;
        }
    };

    if let Some(stdout) = child.stdout.take() {
        let reader = BufReader::new(stdout);
        for line in reader.lines().flatten() {
            // Simple heuristic parser: when avahi-browse prints a resolved line it contains the service type
                if let Some((hostname, ip, arch)) = parse_avahi_line(&line) {
                    println!("Discovered service: host={} ip={} arch={}", hostname, ip, arch);

                    if args.auto_join {
                        let join = args.join_cmd.clone();
                        thread::spawn(move || {
                            let res = Command::new(&join).arg(&ip).arg(&hostname).arg(&arch).spawn();
                            match res {
                                Ok(_) => println!("Spawned join command: {} {} {}", &join, &ip, &hostname),
                                Err(e) => eprintln!("Failed to spawn join command {}: {}", &join, e),
                            }
                        });
                    }
                }
        }
    }

    let _ = child.wait();
}
