use k3s_discover::listener_lib::parse_avahi_line;

#[test]
fn test_parse_normal_line() {
    let line = "=;eth0;IPv4;Node One;_k3s-discover._tcp;local;node-one.local;192.168.1.42;9999;hostname=node-one,ip=192.168.1.42,arch=armv7";
    let parsed = parse_avahi_line(line).expect("should parse");
    assert_eq!(parsed.0, "node-one.local");
    assert_eq!(parsed.1, "192.168.1.42");
    assert_eq!(parsed.2, "armv7");
}

#[test]
fn test_ignore_other_service() {
    let line = "=;eth0;IPv4;foo;_other._tcp;local;host.local;10.0.0.1;9999;";
    assert!(parse_avahi_line(line).is_none());
}
