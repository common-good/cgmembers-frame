REM Generate an assymmetric private key for DKIM and u\cryPGP().
ssh-keygen -t rsa -b 2048 -f dev.privk -m pem -P ""
REN dev.privk.pub dev.pub

REM ssh-keygen -f dev.pub -e -m pem > dev.pubk
REM The above line fails for PHP ssl functions because it puts "RSA" in the comment. Use u\pubKey() instead.
