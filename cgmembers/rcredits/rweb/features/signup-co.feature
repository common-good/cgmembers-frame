Feature: A company signs up to try Common Good using a short form
AS an owner or manager of a company
I WANT to open a Common Good account
SO I can be part of the Common Good Economy

# phone 1 means +1 413.772.0001

Setup:
  Given members:
  | uid | fullName  | phone | email | city  | state | zip   | floor | flags   | pass      | helper |*
  | .ZZA | Abe One  |     1 | a@    | Atown | AK    | 01000 |     0 | member  | %whatever | cgf    |
  | .ZZC | Our Pub  |     3 | c@    | Ctown | CA    | 03000 |     0 | co      |           | .ZZA   |
  | .ZZZ | Zeta Zot |    26 | z@    | Ztown | MS    | 09000 |     0 | co      | zpass     | cgf    |
  And relations:
  | main | other | permission | otherNum |*
  | .ZZC | .ZZA  | manage     | 1        |
  And member is logged out

Scenario: A company tries to sign up directly
  When member "?" visits page "signup-co"
  Then we show "Open a Company Account" with:
  | open an individual account |

Scenario: Someone wants to open a company account
  When member ".ZZA" visits page "signup-co/relate=1"
  Then we show "Open a Company Account" with:
  | Account Type  | |
  | Company       | |
  | Legal Name    | |
  | Email         | |
  | Postal Code   | |
  | Company Phone | |
  | Selling       | |
  | Sell Credit   | |
  | Federal ID    | |
  | Founded       | |
  | Referred By   | |
  | Need Phone    | |
  
Scenario: A company signs up
  Given next random code is "WHATEVER"
  When member ".ZZA" completes form "signup-co/relate=1" with values:
  | fullName  | New Co       |**
  | legalName |              |
  | federalId | 04-3849283   |
  | dob       | %mdY-3y      |
  | zip       | 01004        |
  | phone     | 413-253-0004 |
  | email     | d@           |
  | selling   | fish         |
  | sellCg    | 1            |
  | source    | thither      |
  | coType    | LLC          |
  | needPhone | 1            |
  Then members:
  | uid       | .AAA         |**
  | fullName  | New Co       |
  | legalName | New Co       |
  | federalId | 043849283    |
  | dob       | %daystart-3y |
  | zip       | 01004        |
  | phone     | +14132530004 |
  | email     | d@           |
  | selling   | fish         |
  | source    | thither      |
  | coType    | LLC          |
  And company flags:
  | uid  | coFlags     |*
  | .AAA | sellcg      |
  And relations:
  | main | other | permission |*
  | .AAA | .ZZA  | manage     |
  And invoices:
  | payer | payee | amount         | purpose           |*
  | .AAA  | cgf   | %EQUIP_DEPOSIT | equipment deposit |
  And we email "verify" to member "d@" with subs:
  | fullName | qid      | site      | code     | pass       |*
  | New Co   | NEWAAA-A | %BASE_URL | WHATEVER | co nonpass |
  And we show "Verify Your Email Address"
  And we say "status": "info saved|step completed"
  And we say "status": "refundable deposit"
  And member ".AAA" steps left "verifyemail fund contact backing photo donate company crumbs discount"
  And members have:
  | uid  | signed | signedBy |*
  | .AAA | %today | Abe One  |

  Given member is logged out
  When member "?" visits page "settings/verifyemail/qid=NEWAAA-A&verify=1&code=WHATEVER"
  Then we show "Welcome to %PROJECT" with:
  | Account |
  | Password |
  And we say "status": "verified email"
  And we say "status": "info saved|step completed"
  And we say "status": "continue co setup"
  And member ".AAA" steps left "fund contact backing photo donate company crumbs discount"

Scenario: A company verifies email while signed in
  Given member ".ZZC" has "co" steps done: "signup fund contact backing photo donate company crumbs discount"
  And members have:
  | uid  | flags  |*
  | .ZZA | ok     |
  And member ".ZZC" one-time password is "WHATEVER"
  When member "C:A" visits page "settings/verifyemail/qid=NEWZZC-A&verify=1&code=WHATEVER"
  Then we show "You: Our Pub"
  And we say "status": "verified email"
  And we say "status": "info saved|ok complete|co complete|join thanks"
  And member ".ZZC" steps left ""
  And members have:
  | uid  | flags             |*
  | .ZZC | co,ok,member,ided |
  
Scenario: A company supplies company information
  Given member ".ZZC" has "co" steps done: "signup verifyemail fund contact backing photo donate"
  And members have:
  | uid  | website   | selling |*
  | .ZZC | ourpub.co | drinks  |
  When member "C:A" visits page "settings/company"
  Then we show "Company Information" with:
  | CGPay Button    | |
  | Photo           | |
  | Company name    | Our Pub |
  | Private         | |
  | Categories      | |
  | Selling         | drinks |
  | Short Desc      | |
  | Employees       | |
  | Annual Gross    | |
  | Founded         | |
  | Website         | ourpub.co |
  | Describe        | |
  | App permissions | |
  | Nudge Every     | |
  | Tips            | |

  When member "C:A" completes form "settings/company" with values:
  | fullName    | Our Pub |**
  | private     | 1 |
  | categories  | 0=>%CAT_FOOD, 1=>%CAT_RETAIL |
  | selling     | ale |
  | shortDesc   | bar |
  | employees   | 3 |
  | gross       | $250,000 |
  | dob         | %mdY-9y |
  | website     | Rpub.com |
  | description | really good ale |
  | can         | 0=>0, 0=>2 |
  | staleNudge  | 8 |
  | tips        | 1 |
  Then members have:
  | uid         | .ZZC |**
  | fullName    | Our Pub  |
  | selling     | ale |
  | shortDesc   | bar |
  | employees   | 3 |
  | gross       | 250000 |
  | dob         | %daystart-9y |
  | website     | Rpub.com |
  | description | really good ale |
  | staleNudge  | 8 |
  And company flags:
  | uid  | coFlags     |*
  | .ZZC | private,tip |
  And we show "Share When You Receive" with:
  | Crumbs |
  And we say "status": "info saved|step completed"
  And member ".ZZC" steps left "crumbs discount"

Scenario: A company supplies crumbs choices
  Given member ".ZZC" has "co" steps done: "signup verifyemail fund contact backing photo donate company"
  When member "C:A" completes form "community/crumbs" with values:
  | crumbs | 3 |**
  Then members have:
  | uid  | crumbs |*
  | .ZZC | .03    |
  And we show "Get Your Customers Signed Up"
  And we say "status": "info saved|step completed"
  And member ".ZZC" steps left "discount"

Scenario: A company account manager creates a discount
  Given member ".ZZC" has "co" steps done: "signup verifyemail fund contact backing photo donate company crumbs"
  And members have:
  | uid  | task   |*
  | .ZZC | co     |
  And members have:
  | uid  | flags     |*
  | .ZZA | ok,member |
  When member "C:A" completes form "community/discount" with values:
  | amount | minimum | start | end     | useMax | type     |*
  |     20 |     120 | %mdY  | %mdY+3m |      3 | discount |
  Then these "coupons":
  | coupid | fromId | amount | useMax | flags | start      | end                         |*
  |      1 |   .ZZC |     20 |      3 |       | %daystart  | %(%daystart+3m+%DAY_SECS-1) |
  And we say "status": "Your discount was created successfully."
  And we tell ".ZZC" CO "New Coupons!" with subs:
  | quid      | .ZZC |**
  | type      | discount |
  | amount    | 20 |
  | minimum   | 120 |
  | useMax    | 3 |
  | purpose   | on your purchase of $120 or more |
  | start     | %daystart |
  | end       | %(%daystart+3m+%DAY_SECS-1) |
  | automatic | 1 |
  | company   | Our Pub |
  | gift      | |
  | forOnly   | |
  And we show "You: Our Pub" with:
  | Balance: | $0 |
  And without:
  | Finish |
  And we say "status": "info saved|ok complete|co complete|join thanks"
  And member ".ZZC" steps left ""
  And we tell ".ZZC" CO "New Member (Our Pub)" with subs:
  | quid | status |*
  | .ZZC | member |

Scenario: An unverified company agent completes a company account
  Given member ".ZZC" has "co" steps done: "signup verifyemail fund contact backing photo donate company discount"
  And members have:
  | uid  | flags  |*
  | .ZZA |        |
  When member "C:A" completes form "community/crumbs" with values:
  | crumbs | 3 |**
  Then we say "status": "info saved"
  And we say "status": "tentative complete|co complete"
  And member ".ZZC" steps left ""