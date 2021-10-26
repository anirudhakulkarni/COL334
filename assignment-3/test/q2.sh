# call ./waf --run "scratch/Second --part=a --rate=rateMbps"
# rate is in list ["2Mbps", "4Mbps", "10Mbps", "20Mbps", "50Mbps"]
# navigate to q3 directory


ratelist=("2Mbps" "4Mbps" "10Mbps" "20Mbps" "50Mbps")
for rate in "${ratelist[@]}"
do
    echo "Running with rate $rate"
    ./waf --run "scratch/Second --part=a --rate=$rate"
done


ratelist2=("0.5Mbps" "1Mbps" "2Mbps" "4Mbps" "10Mbps")
for rate in "${ratelist2[@]}"
do
    echo "Running with rate $rate"
    ./waf --run "scratch/Second --part=b --rate=$rate"
done
