use std::net::{UdpSocket, Ipv4Addr, SocketAddrV4};
use std::time::Duration;
use std::thread;

use k3s_discover::send_multicast;

#[test]
fn test_send_and_receive_multicast() {
    // bind to mDNS port and join multicast group
    let recv = UdpSocket::bind(("0.0.0.0", 5353)).expect("bind");
    recv.set_read_timeout(Some(Duration::from_secs(3))).unwrap();
    let mcast = Ipv4Addr::new(224,0,0,251);
    recv.join_multicast_v4(&mcast, &Ipv4Addr::UNSPECIFIED).unwrap();

    let payload = b"integration-test-payload";

    thread::spawn(move || {
        std::thread::sleep(Duration::from_millis(200));
        let _ = send_multicast(payload);
    });

    let mut buf = [0u8; 1500];
    let (n, _src) = recv.recv_from(&mut buf).expect("recv");
    assert_eq!(&buf[..n], payload);
}
