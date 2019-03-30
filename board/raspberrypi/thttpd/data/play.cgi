#!/bin/bash
# 10.10.60.103:8888/play.cgi?x=0&y=0&w=700&h=400&ip=239.100.0.2&port=10149
# 10.10.60.103:8888/play.cgi?x=0&y=0&w=700&h=400&ip=239.100.0.2&port=10149
echo "Content-type: text/plain"
echo ""
kill $(ps aux | grep 'omxplayer' | awk '{print $1}')
eval $(echo ${QUERY_STRING//&/;})
if [ -n "$x" ] && [ -n "$y" ] && [ -n "$w" ] && [ -n "$h" ] && [ -n "$ip" ] && [ -n "$port" ]; then
    echo $x
    echo $y
    echo $w
    echo $h
    echo $ip
    echo $port
    omxplayer --live --win $x,$y,$w,$h udp://@$ip:$port  &>/var/www/logs/omx.log </dev/null &
fi
echo "DONE"
