# lanzaServidor.sh
# Lanza el servidor que es un daemon y varios clientes
# las ordenes están en un fichero que se pasa como tercer parámetro
./servidor
./clientcp localhost ordenes1.txt &
./clientcp localhost ordenes2.txt &
./clientcp localhost ordenes3.txt &
./clientudp localhost ordenes1.txt &
./clientudp localhost ordenes2.txt &
./clientudp localhost ordenes3.txt &
 
