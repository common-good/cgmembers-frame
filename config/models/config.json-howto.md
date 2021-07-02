# How to set up config.json

Database values in config.json will work, unchanged, if you name your database and database user (and password) accordingly.
BUT NOTE!: These values MUST be changed on a production server. In particular, the database password should be very strong and each of the encryption methods should include "aes-256-ctr" or an equivalent alternative. Methods for "V" encryption should also include "pgp" (PGP -- SSL paired-key encryption)
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

Each of the elements under encryption provides parameters for encryption serving a specific purpose:

*C*       cookie (last step of encryption is b64)
*H*       photo
*P*       searchable data, including phone numbers and email addresses (last step of encryption is b64)
*R*       regional admin private key for very secure data (the key is to be encrypted in cgAdmin.html on a flash drive)
*S*       standard secure data
*V*       very secure data (SSNs, etc.)

Each of these elements is an array of [method, password, vector]:

*method*   a space-delimited list of encryption methods: xor, rot, scram, b64, pgp, or any openssl_get_cipher_methods string.
*password* an arbitrary base64url-encoded password, at least 32 characters long
*vector*   an arbitrary base64url-encoded initialization vector, at least 16 characters long

And of these values (method, password, vector) can instead be single character meaning copy values from that other element.
For example, ["S", "whatever", "whatever"] means use the same method as for "S" encryption.
The last method in the list for "C" and "P" must be b64.
For maximum security, the method list for "V" (but no other) should include pgp.

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
