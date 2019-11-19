Feature: A user clicks a "Pay With Common Good" button on a participating company's website
AS a member
I WANT to pay for a web purchase
SO I can get stuff or make donations easily.

Setup:
  Given members:
  | uid  | fullName | pass | email | flags                    | zip   | floor |*
  | .ZZA | Abe One  | a1   | a@    | member,ok,confirmed,debt | 01001 |  -100 |
  | .ZZC | Our Pub  | c1   | c@    | member,ok,co,confirmed   | 01003 |     0 |
  And member is logged out

Scenario: A member clicks a Pay With Common Good button
  When member "?" visits page "pay-with-cg/company=NEWZZC&item=food&amount=23"
  Then we show "Hello %PROJECT Member" with:
  | Pay        | $23 to Our Pub |
  |            | for food |
  | Account ID |  |

Scenario: A member submits a Pay With Common Good button payment with account ID
  When member "?" confirms form "pay-with-cg/company=NEWZZC&item=food&amount=23" with values:
  | name   |*
  | NEWZZA |
  Then we say "status": "pay button success"
  And we message "new invoice" to member ".ZZA" with subs:
  | otherName | amount | purpose |*
  | Our Pub   | $23    | food    |
  And invoices:
  | nvid | created | status      | amount | from | to   | for  |*
  |    1 | %today  | %TX_PENDING |     23 | .ZZA | .ZZC | food |

  When member "?" visits page "handle-invoice/nvid=1&toMe=1&code=TESTDOCODE"
  Then we show "Confirm Payment" with:
  | | Pay $23 to Our Pub for food. |
  | Pay | Dispute |

  When member "?" confirms form "handle-invoice/nvid=1&toMe=1&code=TESTDOCODE" with values:
  | op  |*
  | pay |
  Then we say "status": "You paid Our Pub $23."
  And invoices:
  | nvid | created | status | purpose |*
  |    1 | %today  |      1 | food    |
  And transactions:
  | xid | created | amount | from | to   | for                      |*
  |   1 | %today  |     23 | .ZZA | .ZZC | food (Common Good inv#1) |
  
  When member "?" visits page "handle-invoice/nvid=1&toMe=1&code=TESTDOCODE"
  Then we say "error": "already paid"

Scenario: A member clicks a Pay With Common Good button with variable amount
  When member "?" visits page "pay-with-cg/company=NEWZZC&item=food&amount="
  Then we show "Hello %PROJECT Member" with:
  | Pay        | to Our Pub |
  |            | for food |
  | Account ID |  |

Scenario: A member submits a Pay With Common Good button payment with account ID and chosen amount
  When member "?" confirms form "pay-with-cg/company=NEWZZC&item=food&amount=" with values:
  | name   | amount |*
  | NEWZZA |     23 |
  Then we say "status": "pay button success"
  And we message "new invoice" to member ".ZZA" with subs:
  | otherName | amount | purpose |*
  | Our Pub   | $23    | food    |
  And invoices:
  | nvid | created | status      | amount | from | to   | for  |*
  |    1 | %today  | %TX_PENDING |     23 | .ZZA | .ZZC | food |

  When member "?" visits page "handle-invoice/nvid=1&toMe=1&code=TESTDOCODE"
  Then we show "Confirm Payment" with:
  | | Pay $23 to Our Pub for food. |
  | Pay | Dispute |
  
Scenario: A member signs in with username on the member site
  When member "?" confirms form "signin" with values:
  | name   | pass |*
  | abeone | a1   |
  Then member ".ZZA" is logged in
  And we show "Account Summary"

Scenario: A member signs in with account ID on the member site
  When member "?" confirms form "signin" with values:
  | name    | pass |*
  | newzza | a1   |
  Then member ".ZZA" is logged in
  And we show "Account Summary"

Scenario: A member signs in with email on the member site
  When member "?" confirms form "signin" with values:
  | name          | pass |*
  | a@example.com | a1   |
  Then member ".ZZA" is logged in
  And we show "Account Summary"

Scenario: A member types the wrong password
  When member "?" confirms form "signin" with values:
  | name   | pass |*
  | abeone | a2   |
  Then we say "error": "bad login"

Scenario: A member types an unknown username/ID
  When member "?" confirms form "signin" with values:
  | name  | pass |*
  | bogus | a1   |
  Then we say "error": "bad login"

#.........................................................

Scenario: A member asks for a new password for username
  Given next random code is "wHatEveR"
  When member "?" completes form "settings/password" with values:
  | name   |*
  | abeone |
  Then we email "password-reset" to member "a@example.com" with subs:
  | fullName | site        | name   | code     |*
  | Abe One  | %BASE_URL | abeone | wHatEveR |
  
Scenario: A member asks for a new password for account ID
  Given next random code is "wHatEveR"
  When member "?" completes form "settings/password" with values:
  | name    |*
  | newzza |
  Then we email "password-reset" to member "a@example.com" with subs:
  | fullName | site        | name   | code     |*
  | Abe One  | %BASE_URL | abeone | wHatEveR |
  
Scenario: A member asks for a new password for email
  Given next random code is "wHatEveR"
  When member "?" completes form "settings/password" with values:
  | name          |*
  | a@example.com |
  Then we email "password-reset" to member "a@example.com" with subs:
  | fullName | site        | name   | code     |*
  | Abe One  | %BASE_URL | abeone | wHatEveR |

Scenario: A member asks for a new password for an unknown account
  When member "?" completes form "settings/password" with values:
  | name  |*
  | bogus |
  Then we say "error": "bad account id"
  
Scenario: A member asks for a new password for a company
  When member "?" completes form "settings/password" with values:
  | name    |*
  | newzzc |
  Then we say "error": "no co pass" with subs:
  | company |*
  | Our Pub |
  
#.........................................................

Scenario: A member clicks a link to reset password
  Given next random code is "wHatEveR"
  When member "?" completes form "settings/password" with values:
  | name          |*
  | a@example.com |
  And member "?" visits page "reset/id=abeone&code=wHatEveR"
  Then we show "Choose a New Password"

  When member "?" confirms form "reset/id=abeone&code=wHatEveR" with values:
  | pass1      | pass2      | strong |*
  | %whatever  | %whatever  | 1      |
  Then member ".ZZA" is logged in
  And we show "Account Summary"

  Given member is logged out
  When member "?" confirms form "signin" with values:
  | name   | pass      |*
  | abeone | %whatever |
  Then we show "Account Summary"
  And member ".ZZA" is logged in

Scenario: A member clicks a link to reset password with wrong code
  Given next random code is "wHatEveR"
  And member "?" completes form "settings/password" with values:
  | name          |*
  | a@example.com |
  When member "?" visits page "reset/id=abeone&code=NOTwHatEveR"
  Then we say "error": "bad login"
  And we show "Miscellaneous"

Scenario: A member clicks a link to reset password for unknown account
  Given next random code is "wHatEveR"
  And member "?" completes form "settings/password" with values:
  | name          |*
  | a@example.com |
  When member "?" visits page "reset/id=abeone&code=NOTwHatEveR"
  Then we say "error": "bad login"
  And we show "Miscellaneous"