Common Good&trade; Democratic Economics System
=====================================================================

Copyright &copy; 2019 Society to Benefit Everyone, Inc., dba Common Good

This community-centered payment system is designed to provide greater local control of economics, so we can make our communities everything we want them to be.

The companion software project, Common Good POS, works very much like credit card processing apps for swipe cards. Except instead of a swipe card reader you use your smartphone's camera to scan a QR code on the customer's Common Good Card or phone.

Installation
------------

### Prerequisites

1. Install Nginx or Apache, MariaDB 10.2+ or MySQL 8+, and PHP 7+ with openssl and imagick modules.
   1. Mac: As of May 2021, MAMP does not support MySQL 8, so it is recommended to use [Homebrew](https://brew.sh/) packages (otherwise migrations will not run). 
   2. Windows: Download the [imagick module for PHP](https://pecl.php.net/package/imagick) and put it in the PHP extension (“ext”) folder.
2. If you’re going to work on the biodiesel pump interface, install Perl.
4. Install git and [git-flow](https://github.com/nvie/gitflow/wiki/Installation). 
5. Clone the repository. 

### Database and config files

1. Create an empty database (default name "cg").
2. Create a db user (default "root" with empty password). Grant the user all privileges to the database and SUPER privileges on the server. 
3. Copy `config.json` and `phinx.json` from `cgmembers/rcredits/misc/config-models/` to `config/`.
4. Edit `config.json` to match your environment. 
5. Import the startup database by executing `recreate.bat` or `recreate.sh` in the app root (requires using default values), or manually:
   1. Import `db/startup.sql`.
   2. Run `vendor/robmorgan/phinx/bin/phinx migrate -c config/phinx.json -e development`.
