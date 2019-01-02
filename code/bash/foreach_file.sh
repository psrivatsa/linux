#!/bin/bash
for filename in ./*.pcap; do
   echo "Processing pcap file :: " $filename
   ./scripts/pcap_parser $filename 0 >> $1
done
