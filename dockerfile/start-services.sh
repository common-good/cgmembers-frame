#!/bin/bash
# start-services.sh

# Start Apache in the background
/usr/sbin/apache2ctl -D FOREGROUND &

# Start your Node.js application (adjust the path as needed)
cd /root/cgpay-node-api
source $NVM_DIR/nvm.sh
nvm install 20.10.0 && nvm use 20.10.0 && npm install && npm install dotenv --save && npm start &

# Keep the container running and show logs
tail -f /var/log/apache2/access.log /var/log/apache2/error.log