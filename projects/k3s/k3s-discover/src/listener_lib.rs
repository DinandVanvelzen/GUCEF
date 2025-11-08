use std::collections::HashMap;

/// Parse a single line from `avahi-browse -rt` resolved output.
/// Expected semicolon-separated format (example):
/// =;eth0;IPv4;My Service;_k3s-discover._tcp;local;hostname.local;192.168.1.10;9999;hostname=foo,ip=1.2.3.4,arch=armv7
/// Returns (hostname, ip, arch) if parse successful.
pub fn parse_avahi_line(line: &str) -> Option<(String, String, String)> {
    if !line.contains("_k3s-discover._tcp") {
        return None;
    }

    let parts: Vec<&str> = line.split(';').collect();
    if parts.len() < 10 {
        return None;
    }

    let hostname = parts[6].trim().to_string();
    let ip = parts[7].trim().to_string();
    let txt = parts[9].trim();

    // parse txt as comma-separated key=value
    let mut map = HashMap::new();
    if !txt.is_empty() {
        for pair in txt.split(',') {
            if let Some(eq) = pair.find('=') {
                let k = pair[..eq].trim();
                let v = pair[eq+1..].trim();
                map.insert(k.to_string(), v.to_string());
            }
        }
    }

    let arch = map.get("arch").cloned().unwrap_or_default();

    Some((hostname, ip, arch))
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_parse_avahi_line() {
        let line = "=;eth0;IPv4;Node One;_k3s-discover._tcp;local;node-one.local;192.168.1.42;9999;hostname=node-one,ip=192.168.1.42,arch=armv7";
        let parsed = parse_avahi_line(line).expect("should parse");
        assert_eq!(parsed.0, "node-one.local");
        assert_eq!(parsed.1, "192.168.1.42");
        assert_eq!(parsed.2, "armv7");
    }
}
