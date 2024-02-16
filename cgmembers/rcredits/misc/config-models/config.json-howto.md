# How to set up config.json

Database values in config.json will work, unchanged, if you name your root directory, database, and database user (and password) accordingly.
BUT NOTE!: These values MUST be changed on a production server. In particular, the database password should be very strong 
and each of the encryption methods should include "aes-256-ctr" or an equivalent alternative. 
Methods for "V" encryption should also include "pp" (SSL paired-key encryption).
Features that need not work (and will not work) in a development environment are: SMTP, SSN Verification, and DKIM.
Geocoding will work if you sign up with opencagedata.com and get a key.

### Database

*name*    database name
*driver*  database driver (for example, mysql)
*host*    database connection host name (normally localhost)
*port*    database connection port (normally 3306)
*user*    database user name
*pass*    password for that user
*salt*    64 arbitrary hex digits
*word*    occasional padding for encryption -- 32 or 64 hex digits (add this to a short or guessable value before encrypting)

### Encryption

Each of the elements under encryption provides parameters for exncryption serving a specific purpose:

*C*       cookie (last step of encryption is b64)
*H*       photo
*P*       searchable data, including phone numbers and email addresses (last step of encryption is b64)
*R*       regional admin private key for very secure data (the key is to be encrypted in cgAdmin.html on a flash drive)
*S*       standard secure data
*V*       very secure data (SSNs, etc.)

Each of these elements is an array of [method, password, vector]:

*method*   a space-delimited list of encryption methods: xor, rot, scram, b64, pp (for "V" only), or any openssl_get_cipher_methods string.
*password* an arbitrary base64url-encoded password, at least 32 characters long
*vector*   an arbitrary base64url-encoded initialization vector, at least 16 characters long

And of these values (method, password, vector) can instead be single character meaning copy values from that other element.
For example, ["S", "whatever", "whatever"] means use the same method as for "S" encryption.
The last method in the list for "C" and "P" must be b64.
For maximum security, the method list for "V" should include pp.

### SMTP

*cgSmtpServer* SMTP server address (generally not needed for development)
*cgSmtpTls*    tls or ssl -- encryption method for swiftmailer
*cgSmtpPort*   SMTP port (normally 587)
*cgSmtpUser*   SMTP username
*cgSmtpPass*   SMTP password

### Social Security Number Verification (for example by nationalpublicdata.com)

*ssnUser*      username for SSN verification service
*ssnPass*      password for SSN verification service
*ssnRequest*   template for REST request for SSN verification

### Other
  
*stage*        development, staging, game, or production
*baseUrl*      URL of the application
*promoUrl*     URL of the Common Good Promotional site
*coApiUrl*     URL of the APIs for co-branding partners
*cgfEmail*     email address for contacting Common Good

*dkimPrivate*  private key for email encryption, base64 encoded with EOLs replaced by spaces (not used in development)
*dkimPublic*   public key for email encryption, base64 encoded with EOLs replaced by spaces (not used, here for reference)
*geocodeKey*   account identifier for opencagedata.com geo-coding service
*inviteKey*    arbitrary string of 0s and 1s, for generating invitation codes (must be 10101 for development tests to work)

=================================================================================

### Customizing the Encryption Keys and Passwords

The example config.json file uses the private/public key pair stored in misc/devkeys/cg.privk (and .pubk) for the "privk" value (Common Good's private key for identifying other devices) and the pair stored in misc/devkeys/v.privk (and .pubk) for "V" encryption. The Common Good system actually uses SSL asymmetric encryption/decryption rather than PGP. For development, there is no need to change anything. But for any system using real data, you will need to choose new keys. Here's how:

## Make new keys

* Open cgmembers/rcredits/misc/makeKey.php in your browser.
* Set the first text box to the path to openssl.cnf on your system, which, depending on your system, you may find in /apache/bin, /etc/ssl, /etc/openssl, apache\apache2.4.46\conf, somewhere inside your WAMP/XAMPP/LAMP/MAMP package, or in a system library. If you can’t find it, you may need to install openssl using your system’s package manager. 
* Choose no more than about 2048 bits. 
* Submit the form. Copy the result to the clipboard and to an appropriate .privk file (cg.privk or v.privk) and store it somewhere secure (NOT in the repo!). Perhaps store it also as a special value in a password management service.

## Set the values for encryption methods

Each encryption method entry has 3 members: methods, password, and initialization vector (IV). For the first (methods) value of any encryption type other than “V”, you can use any combination of xor, scram (scramble), rot (rotate), and any method listed by openssl_get_cipher_methods(), separated by spaces. For example: “xor bf-ofb scram”. Each of the encryption methods will be applied in turn, using the password and IV. Each encryption password and IV should be a different random string (using all 8 bits of each character), base64url-encoded using either + and / or with those characters replaced by - and _ (minus and underline). You can generate them by browsing to cgmembers/rcredits/misc/makePw.php.

## Set the value for "privk"

Simply copy everthing but the header and footer from your cg.privk file into config.json as the value of "privk".

## Set special values for "V" encryption

* The first value on the “V” line should include a special method “pp”. This method uses asymmetric public/private encryption/decryption, so social security numbers and bank account numbers (etc.) can be encrypted by anyone but only decrypted by a SuperAdmin. Do not use the "pp" method anywhere else.
* Browse to cgmembers and sign in with username admin -- any non-empty password will do. If you get “ORDER BY clause is not in SELECT list” crash error, set sql-mode in my.ini file to “” (empty string) (my.ini can be opened from Wamp console under MySql) 
* Get the “V” password. Type this in the PHP box and press “Execute Code”:

    $k = <key>;
	echo u\b64encode(u\unfmtKey(u\pubKey($k)));

where <key> is the entire contents of the v.privk file you created above, surrounded by single quotes. Paste the result into config/config.json as the password for “V” encryption (the 2nd of the 3 values on the “V” line).

* Set vKeyE and vKeyPw. For each superAdmin, part of the "V" private key is stored on the server (vKeyE) and part is given to them as a password (vKeyPw) -- either as text or as a QR code. To create a vKeyE/vKeyPw pair for user admin (recommended to get started), type this in the box and press “Execute Code”:

    $k = <key>;
    echo a(1)->makeVKeyE($k, NULL, FALSE);

Where <key> is the same key as above. Paste the result into cgAdmin.html as the default value of the vKeyPw input. The value for vKeyE is stored automatically in the admins table.
