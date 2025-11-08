use std::io;
use std::net::{TcpStream, ToSocketAddrs};
use std::process::{Child, Command};
use std::str;
use std::thread;
use std::time::{Duration, SystemTime, UNIX_EPOCH};

use hostname::get;
use k3s_discover::{pick_local_ip, send_multicast};
use serde::Serialize;

const MDNS_IPV4: &str = "224.0.0.251:5353";
const DEFAULT_INTERVAL_SECS: u64 = 60;
const DEFAULT_CONNECT_TIMEOUT_SECS: u64 = 3;

#[derive(Serialize)]
struct Candidate<'a> {
    hostname: &'a str,
    ip: &'a str,
    arch: &'a str,
    ts: u64,
    note: &'a str,
}

fn main() -> io::Result<()> {
    println!("k3s-discover starting");

    let interval = std::env::var("K3S_DISCOVER_INTERVAL")
        .ok()
        .and_then(|s| s.parse::<u64>().ok())
        .unwrap_or(DEFAULT_INTERVAL_SECS);

    // Determine hostname and local IP to advertise
    let hostname = get()
        .ok()
        .and_then(|h| h.into_string().ok())
        .unwrap_or_else(|| "unknown".into());

    let local_ip = pick_local_ip().unwrap_or_else(|| "0.0.0.0".to_string());

    let arch = std::env::consts::ARCH;

    // Determine k3s server to check
    let server = find_k3s_server().unwrap_or_else(|| "127.0.0.1:6443".to_string());
    eprintln!("checking k3s server: {}", server);

    loop {
        let connected = check_connectivity(&server, Duration::from_secs(DEFAULT_CONNECT_TIMEOUT_SECS));

        if !connected {
            eprintln!("k3s not reachable, advertising candidate via mDNS/DNS-SD if possible");

            // Build TXT records
            let now = SystemTime::now().duration_since(UNIX_EPOCH).unwrap_or_default().as_secs();
            let txt_records = vec![
                format!("hostname={}", hostname),
                format!("ip={}", local_ip),
                format!("arch={}", arch),
                format!("ts={}", now),
            ];

            // Try to use avahi-publish-service if available for proper DNS-SD
            let mut avahi_child: Option<Child> = None;
            if let Ok(child) = start_avahi_publish(&hostname, &txt_records) {
                avahi_child = Some(child);
            } else {
                // fallback to raw JSON multicast
                let payload = Candidate {
                    hostname: &hostname,
                    ip: &local_ip,
                    arch,
                    ts: now,
                    note: "k3s-candidate",
                };
                if let Ok(p) = serde_json::to_vec(&payload) {
                    let _ = send_multicast(&p);
                }
            }

            // Advertise with a backoff loop until connectivity returns
            let mut backoff = interval;
            loop {
                thread::sleep(Duration::from_secs(backoff));
                // re-check connectivity
                if check_connectivity(&server, Duration::from_secs(DEFAULT_CONNECT_TIMEOUT_SECS)) {
                    eprintln!("k3s reachable again");
                    // stop avahi publisher if running
                    if let Some(mut c) = avahi_child {
                        let _ = c.kill();
                        let _ = c.wait();
                    }
                    break;
                }

                // send again (either avahi is still running or we resend multicast)
                if avahi_child.is_none() {
                    let now = SystemTime::now().duration_since(UNIX_EPOCH).unwrap_or_default().as_secs();
                    let payload = Candidate {
                        hostname: &hostname,
                        ip: &local_ip,
                        arch,
                        ts: now,
                        note: "k3s-candidate",
                    };
                    if let Ok(p) = serde_json::to_vec(&payload) {
                        let _ = send_multicast(&p);
                    }
                }

                // exponential backoff, capped
                backoff = std::cmp::min(backoff.saturating_mul(2), 600);
            }
        }

        thread::sleep(Duration::from_secs(interval));
    }
}

fn start_avahi_publish(hostname: &str, txt: &[String]) -> Result<Child, ()> {
    // avahi-publish-service <name> <type> <port> [TXT=...]
    // Example: avahi-publish-service "node-host" _k3s-discover._tcp 9999 hostname=foo ip=1.2.3.4
    let svc_name = format!("k3s-candidate-{}", hostname);
    let svc_type = "_k3s-discover._tcp";
    let port = "9999";

    let mut cmd = Command::new("avahi-publish-service");
    cmd.arg(&svc_name).arg(svc_type).arg(port);
    for entry in txt.iter() {
        cmd.arg(entry);
    }

    match cmd.spawn() {
        Ok(child) => Ok(child),
        Err(_) => Err(()),
    }
}

// send_multicast and pick_local_ip are provided by the library crate (src/lib.rs)

fn check_connectivity(server: &str, timeout: Duration) -> bool {
    // server may be host:port or a URL. Try to parse host:port
    let addrs = server.to_socket_addrs();
    if let Ok(mut iter) = addrs {
        if let Some(addr) = iter.next() {
            return TcpStream::connect_timeout(&addr, timeout).is_ok();
        }
    }

    // If parsing failed, try to extract host:port manually
    if let Some((host, port)) = parse_host_port(server) {
        let addr = format!("{}:{}", host, port);
        if let Ok(mut iter) = addr.to_socket_addrs() {
            if let Some(sa) = iter.next() {
                return TcpStream::connect_timeout(&sa, timeout).is_ok();
            }
        }
    }

    false
}

fn parse_host_port(s: &str) -> Option<(String, u16)> {
    // Accept formats like https://1.2.3.4:6443 or 1.2.3.4:6443
    let trimmed = s.trim();
    let t = if let Some(pos) = trimmed.find("//") {
        &trimmed[pos + 2..]
    } else {
        trimmed
    };

    let mut parts = t.split('/');
    if let Some(first) = parts.next() {
        if let Some(colon) = first.rfind(':') {
            let host = first[..colon].to_string();
            if let Ok(port) = first[colon + 1..].parse::<u16>() {
                return Some((host, port));
            }
        }
    }

    None
}

fn find_k3s_server() -> Option<String> {
    // Try systemd units 'k3s' and 'k3s-agent'
    for unit in ["k3s", "k3s-agent"].iter() {
        // systemctl show -p ExecStart <unit>
        let out = Command::new("systemctl").arg("show").arg("-p").arg("ExecStart").arg(unit).output();
        if let Ok(o) = out {
            if o.status.success() {
                if let Ok(s) = String::from_utf8(o.stdout) {
                    if let Some((host, port)) = parse_server_from_execstart(&s) {
                        return Some(format!("{}:{}", host, port));
                    }
                }
            }
        }
    }

    None
}

fn parse_server_from_execstart(s: &str) -> Option<(String, u16)> {
    // naive parse: look for '--server https://host:port' or '--server=https://host:port'
    let lowercase = s.to_lowercase();
    if let Some(pos) = lowercase.find("--server") {
        let sub = &s[pos..];
        // split by spaces
        for token in sub.split_whitespace() {
            if token.starts_with("--server") {
                let eq_split: Vec<&str> = token.splitn(2, '=').collect();
                let val = if eq_split.len() == 2 { eq_split[1] } else { /* maybe next token */ "" };
                let candidate = if !val.is_empty() { val } else {
                    // try to take next token
                    let mut parts = sub.split_whitespace();
                    parts.next(); // skip the --server token
                    parts.next().unwrap_or("")
                };

                if !candidate.is_empty() {
                    if let Some((h, p)) = parse_host_port(candidate) {
                        return Some((h, p));
                    }
                }
            }
        }
    }

    None
}
