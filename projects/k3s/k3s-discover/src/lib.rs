pub mod listener_lib;

use std::io;
use std::net::UdpSocket;
use std::time::Duration;

/// Send raw payload to mDNS IPv4 multicast address (224.0.0.251:5353)
pub fn send_multicast(payload: &[u8]) -> io::Result<()> {
    let socket = UdpSocket::bind("0.0.0.0:0")?;
    socket.set_multicast_ttl_v4(1)?;
    let _ = socket.send_to(payload, "224.0.0.251:5353")?;
    Ok(())
}

/// Try to pick a non-loopback IPv4 address for the host
pub fn pick_local_ip() -> Option<String> {
    if let Ok(ifaces) = get_if_addrs::get_if_addrs() {
        for iface in ifaces {
            if iface.is_loopback() { continue; }
            if let std::net::IpAddr::V4(ipv4) = iface.ip() {
                let s = ipv4.to_string();
                if s.starts_with("169.254.") { continue; }
                return Some(s);
            }
        }
    }

    // Fallback via UDP socket
    if let Ok(sock) = UdpSocket::bind("0.0.0.0:0") {
        if sock.connect("8.8.8.8:80").is_ok() {
            if let Ok(local) = sock.local_addr() {
                return Some(local.ip().to_string());
            }
        }
    }

    None
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::thread;

    #[test]
    fn test_send_multicast_and_receive() -> Result<(), Box<dyn std::error::Error>> {
        // try to bind to mDNS port and join multicast; if it's busy we skip the test to
        // avoid false negatives in environments where mDNS is already in use.
        let recv = match std::net::UdpSocket::bind(("0.0.0.0", 5353)) {
            Ok(s) => s,
            Err(e) => {
                eprintln!("Skipping multicast test; could not bind to 0.0.0.0:5353: {}", e);
                return Ok(());
            }
        };

        recv.set_read_timeout(Some(Duration::from_secs(2)))?;
        recv.set_nonblocking(false)?;
        let mcast_v4 = std::net::Ipv4Addr::new(224,0,0,251);
        recv.join_multicast_v4(&mcast_v4, &std::net::Ipv4Addr::UNSPECIFIED)?;

        let payload = b"hello-test";

        // send in a thread
        thread::spawn(move || {
            std::thread::sleep(Duration::from_millis(100));
            let _ = send_multicast(payload);
        });

        let mut buf = [0u8; 1500];
        let (n, _src) = recv.recv_from(&mut buf)?;
        let got = &buf[..n];
        assert_eq!(got, payload);
        Ok(())
    }
}
