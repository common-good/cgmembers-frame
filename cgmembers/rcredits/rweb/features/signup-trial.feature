Feature: A company signs up to try Common Good using a short form
AS an owner or manager of a company
I WANT to open a Common Good account
SO I can be part of the Common Good Economy

Setup:
  Given members:
  | uid  | fullName | acctType    | flags      | created  | federalId | postalAddr          |*
  | .ZZZ | Zeta Zot | personal    | ok         | 99654321 | 123456789 | 26Z, Zton, CA 98765 |
  And member is logged out
Skip
Scenario: Someone opens a trial company account
  Given next random code is "WHATEVER"
  When member "?" confirms form "trial" with values:
  | contact | fullName | email | phone        | zip   | ownPhone | qid    |*
  | Abe One | Coco Co  | a@    | 413-253-0000 | 01002 |        1 |        |
  Then members:
  | uid       | .AAA           |**
  | fullName  | Coco Co        |
  | legalName | %CGF_LEGALNAME |
  | federalId | %CGF_EIN       |
  | email     | a@             |
  | phone     | +14132530000   |
  | zip       | 01002          |
  | country   | US             |
  | state     | MA             |
  | city      | Amherst        |
  | flags     | confirmed co depends |
  | helper    | cgf            |
  | contact   | Abe One        |
  And we email "verify-trial" to member "a@" with subs:
  | fullName | name   | quid   | site      | code     |*
  | Coco Co  | cococo | NEWAAA | %BASE_URL | WHATEVER |
  And member ".AAA" one-time password is set to "WHATEVER"
  And we say "status": "info saved|step completed"
  And we show "Get Your Customers Signed Up" with:
  | Discount: | Amount or Percentage |
Skip
For example $20 or 10%
Minimum:
0
Minimum purchase amount, to get the discount.
Valid until:
02/20/2020
Leave blank for no end.
Limit:
5
uses per member
Leave blank for unlimited uses.

  And steps "discount verify" remain for member ".AAA"

Scenario: A member opens a trial company account
  Given next random code is "WHATEVER"
  When member "?" confirms form "trial" with values:
  | contact | fullName | email | phone        | zip   | ownPhone | qid  |*
  | Abe One | Coco Co  | a@    | 413-253-0000 | 01002 |        1 | .ZZZ |
  Then members:
  | uid       | .AAA           |**
  | fullName  | Coco Co        |
  | legalName | %CGF_LEGALNAME |
  | federalId | %CGF_EIN       |
  | email     | a@             |
  | phone     | +14132530000   |
  | zip       | 01002          |
  | country   | US             |
  | state     | MA             |
  | city      | Amherst        |
  | flags     | confirmed co depends |
  | helper    | .ZZZ           |
  | contact   | Abe One~NEWZZZ |
 
Scenario: A member opens a trial company account without a phone
  Given next random code is "WHATEVER"
  When member "?" confirms form "trial" with values:
  | contact | fullName | email | phone        | zip   | ownPhone | qid  |*
  | Abe One | Coco Co  | a@    | 413-253-0000 | 01002 |        0 | .ZZZ |
  Then these "invoices":
  | nvid | created | status      | amount         | from | to  | for               |*
  |    1 | %today  | %TX_PENDING | %EQUIP_DEPOSIT | .AAA | cgf | equipment deposit |
  And we say "status": "refundable deposit"
Resume
Scenario: A new trial company account manager verifies the email
  And members:
  | uid       | .AAA           |**
  | fullName  | Coco Co        |
  | email     | a@             |
  | flags       | confirmed co depends |
  And member ".AAA" one-time-password is "WHATEVER" expires "%now+7d"
  And steps "discount verify" remain for member ".AAA"
  When member ".AAA" completes form "settings/verify" with values:
  | verify   | pass1      | pass2      | strong |*
  | WHATEVER | %whatever3 | %whatever3 |      1 |
  Then we show "Get Your Customers Signed Up"
  And we say "status": "pass saved|step completed"

Scenario: A trial company account manager creates a discount
  Given members:
  | uid       | .AAA           |**
  | fullName  | Coco Co        |
  | email     | a@             |
  | flags     | confirmed co depends |
  And steps "discount verify" remain for member ".AAA"
  When member ".AAA" completes form "community/discount" with values:
  | amount | minimum | end     | ulimit |*
  |     20 |     120 | %mdY+3m |      3 |
  Then these "coupons":
  | coupid | fromId | amount | ulimit | flags | start      | end                         |*
  |      1 |   .AAA |     20 |      3 |       | %daystart  | %(%daystart+3m+%DAY_SECS-1) |
  And we say "status": "Your discount was created successfully."
  And we show "Verify Your Email Address"
  And steps "verify" remain for member ".AAA"
  
Skip
Scenario: A member makes a payment from a trial comppany account
  Given members:
  | uid       | .AAA           |**
  | fullName  | Coco Co        |
  | email     | a@             |
  | flags     | confirmed co depends ok |
  And balances:
  | uid  | balance |*
  | .AAA | 100     |
  When member ".AAA" confirms form "pay" with values:
  | op  | who      | amount | goods | purpose |*
  | pay | Zeta Zot | 100    | %FOR_GOODS     | labor   |
  Then we say "status": "report tx" with subs:
  | did    | otherName | amount |*
  | paid   | Zeta Zot  | $100   |
  And we notice "new payment" to member ".ZZZ" with subs:
  | created | fullName | otherName | amount | payeePurpose |*
  | %today  | Zeta Zot | Coco Co   | $100   | labor        |
  And transactions:
  | xid | created | amount | from  | to   | purpose      | taking |*
  |   1 | %today  |    100 | .AAA  | .ZZZ | labor        | 0      |
  And balances:
  | uid  | balance |*
  | .AAA |       0 |
  | .ZZZ |     100 |
