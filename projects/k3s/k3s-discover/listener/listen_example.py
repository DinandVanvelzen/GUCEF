#!/usr/bin/env python3
"""
DNS-SD listener using zeroconf that listens for services of type _k3s-discover._tcp.local.
When a service appears it resolves TXT records and addresses and optionally runs a join command.

Usage:
  python3 listener/listen_example.py --auto-join --join-cmd /usr/local/bin/k3s-discover-join.sh
"""
import argparse
import subprocess
import time
from zeroconf import Zeroconf, ServiceBrowser, ServiceStateChange


class K3SListener:
    def __init__(self, zeroconf, auto_join=False, join_cmd=None):
        self.zeroconf = zeroconf
        self.auto_join = auto_join
        self.join_cmd = join_cmd

    def remove_service(self, zeroconf, type, name):
        print(f"Service removed: {name}")

    def add_service(self, zeroconf, type, name):
        info = zeroconf.get_service_info(type, name)
        if not info:
            print(f"Service {name} added but no info")
            return

        addresses = []
        for addr_bytes in info.addresses:
            if len(addr_bytes) == 4:
                addresses.append(".".join(str(b) for b in addr_bytes))
        props = {k.decode('utf-8'): v.decode('utf-8') for k, v in info.properties.items()} if info.properties else {}
        print(f"Discovered {name} at {addresses} props={props}")

        if self.auto_join and self.join_cmd and addresses:
            ip = addresses[0]
            hostname = props.get('hostname', name)
            arch = props.get('arch', '')
            try:
                subprocess.Popen([self.join_cmd, ip, hostname, arch])
                print(f"Started join command for {ip}")
            except Exception as e:
                print(f"Failed to start join command: {e}")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--auto-join', action='store_true', help='Automatically run join command when node discovered')
    parser.add_argument('--join-cmd', default='/usr/local/bin/k3s-discover-join.sh', help='Command to run for joining a discovered node')
    args = parser.parse_args()

    zeroconf = Zeroconf()
    listener = K3SListener(zeroconf, auto_join=args.auto_join, join_cmd=args.join_cmd)
    service_type = '_k3s-discover._tcp.local.'
    browser = ServiceBrowser(zeroconf, service_type, listener)

    print('Listening for DNS-SD service type', service_type)
    try:
        while True:
            time.sleep(0.1)
    except KeyboardInterrupt:
        pass
    finally:
        zeroconf.close()


if __name__ == '__main__':
    main()
