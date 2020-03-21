Feature: A company signs up to try Common Good using a short form
AS an owner or manager of a company
I WANT to open a Common Good account
SO I can be part of the Common Good Economy

# phone 1 means +1 413.772.0001

Setup:
  Given members:
  | uid | fullName  | phone | email | city  | state | zip   | floor | flags   | pass      |*
  | .ZZA | Abe One  |     1 | a@    | Atown | AK    | 01000 |     0 | member  | %whatever |
  | .ZZC | Our Pub  |     3 | c@    | Ctown | CA    | 03000 |     0 | co      |           |
  | .ZZZ | Zeta Zot |    26 | z@    | Ztown | MS    | 09000 |     0 | co      | zpass     |
  And relations:
  | main | other | permission |*
  | .ZZC | .ZZA  | manage     |
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
  | Federal ID    | |
  | Founded       | |
  | Referred By   | |
  | Own Phone     | |
  
Scenario: A company signs up
  Given next random code is "WHATEVER"
  When member ".ZZA" completes form "signup-co/relate=1" with values:
  | fullName | legalName | federalId | dob      | zip   | phone        | email | selling | source  | coType  | ownPhone |*
  | New Co   |           | 04-3849283 | %mdY-3y | 01004 | 413-253-0004 | d@    | fish    | thither | %CO_LLC |        0 |
  Then members:
  | uid  | fullName | legalName | federalId | dob          | zip   | phone        | email | coType  | selling |*
  | .AAA | New Co   | New Co    | 043849283 | %daystart-3y | 01004 | +14132530004 | d@    | %CO_LLC | fish    |
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
  And member ".AAA" steps left "verifyemail agree contact backing photo donate company tithein discount"

  Given member is logged out
  When member "?" visits page "settings/verifyemail/id=NEWAAA-A&code=WHATEVER&verify=1"
  Then we show "Verified!"
  And member ".AAA" steps left "agree contact backing photo donate company tithein discount"

Scenario: A company supplies company information
  Given member ".ZZC" has "co" steps done: "signup verifyemail agree contact backing photo donate"
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
  | Description     | |
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
  And member ".ZZC" steps left "tithein discount"

Scenario: A company supplies incoming tithe choices
  Given member ".ZZC" has "co" steps done: "signup verifyemail agree contact backing photo donate company"
  When member "C:A" completes form "settings/tithein" with values:
  | crumbs | 3 |**
  Then members have:
  | uid  | crumbs |*
  | .ZZC | .03    |
  And we show "Get Your Customers Signed Up"
  And we say "status": "info saved|step completed"
  And member ".ZZC" steps left "discount"

Scenario: A company account manager creates a discount
  Given member ".ZZC" has "co" steps done: "signup verifyemail agree contact backing photo donate company tithein"
  When member ".ZZC" completes form "community/discount" with values:
  | amount | minimum | start | end     | ulimit | type     |*
  |     20 |     120 | %mdY  | %mdY+3m |      3 | discount |
  Then these "coupons":
  | coupid | fromId | amount | ulimit | flags | start      | end                         |*
  |      1 |   .ZZC |     20 |      3 |       | %daystart  | %(%daystart+3m+%DAY_SECS-1) |
  And we say "status": "Your discount was created successfully."
  And we tell ".ZZC" CO "New Coupons!" with subs:
  | quid      | .ZZC |**
  | type      | discount |
  | amount    | 20 |
  | minimum   | 120 |
  | ulimit    | 3 |
  | on        | on your purchase of $120 or more |
  | start     | %daystart |
  | end       | %(%daystart+3m+%DAY_SECS-1) |
  | automatic | 1 |
  | company   | Our Pub |
  | gift      | |
  | forOnly   | |
  And we show "Account Summary" with:
  | Next Steps |
  | Invite |
  | Give   |
  And we say "status": "info saved|setup complete|company approval|join thanks"
  And member ".ZZC" steps left ""
  And we tell ".ZZC" CO "New Member (Our Pub)" with subs:
  | quid | status |*
  | .ZZC | member |
