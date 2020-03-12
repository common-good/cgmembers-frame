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
  | For        | food |
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
  | For        | food |
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
