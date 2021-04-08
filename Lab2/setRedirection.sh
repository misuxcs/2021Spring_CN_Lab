if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    # redirection
    echo -n "[INFO] Redirecting..."
    iptables -t nat -A PREROUTING -p tcp --dport 80 -j DNAT --to 10.0.2.15:9090
    iptables -t nat -A PREROUTING -p tcp --dport 443 -j DNAT --to 10.0.2.15:9090
    
    # update firewall rules
    echo -n "[INFO] Updating firewall rules..."
    iptables -t nat -I PREROUTING 1 -p all -d 10.42.0.105 -j ACCEPT
    iptables -t nat -I PREROUTING 2 -p all -s 10.42.0.105 -j ACCEPT

elif [[ "$OSTYPE" == "darwin"* ]]; then
    # Mac OSX
    echo -n "[INFO] This is mac!"
else 
    echo -n "[ERROR] $OSTYPE is not support!"
fi
