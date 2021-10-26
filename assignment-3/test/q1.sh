# TcpNewReno, TcpHighSpeed, TcpVeno, TcpVegas
protocolList=("NewReno" "HighSpeed" "Veno" "Vegas")
for prot in "${protocolList[@]}"
do
    ./waf --run "scratch/First --type=$prot"
done