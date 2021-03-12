#/bin/sh

# Certificate Authority (CA)
openssl genrsa -passout pass:qwerty -out ca-secret.key 4096
openssl rsa -passin pass:qwerty -in ca-secret.key -out ca.key
openssl req -new -x509 -days 3650 -subj '/C=SP/ST=Barueri/L=Minsk/O=Smattch Server/OU=Smattch Server unit/CN=smattch.com' -key ca.key -out ca.crt
openssl pkcs12 -export -passout pass:qwerty -inkey ca.key -in ca.crt -out ca.pfx
openssl pkcs12 -passin pass:qwerty -passout pass:qwerty -in ca.pfx -out ca.pem

openssl genrsa -passout pass:qwerty -out server-secret.key 4096
openssl rsa -passin pass:qwerty -in server-secret.key -out server.key
openssl req -new -subj '/C=SP/ST=Barueri/L=Minsk/O=Smattch Server/OU=Smattch Server unit/CN=smattch.com' -key server.key -out server.csr
openssl x509 -req -days 3650 -in server.csr -CA ca.crt -CAkey ca.key -set_serial 01 -out server.crt
openssl pkcs12 -export -passout pass:qwerty -inkey server.key -in server.crt -out server.pfx
openssl pkcs12 -passin pass:qwerty -passout pass:qwerty -in server.pfx -out server.pem

openssl dhparam -out dh4096.pem 4096