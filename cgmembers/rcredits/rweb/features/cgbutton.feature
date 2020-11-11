Feature: A user clicks a "Pay With Common Good" button on a participating company's website
AS a member
I WANT to pay a member company or individual by clicking a CGPay button
SO I can get stuff, buy credit, or make donations easily.

Setup:
  Given members:
  | uid  | fullName | pass | email | flags                    | zip   | floor | emailCode |*
  | .ZZA | Abe One  | a1   | a@    | member,ok,confirmed,debt | 01001 |  -100 | Aa1       |
  | .ZZB | Bea Two  | b1   | b@    | member,ok,confirmed,debt | 01001 |  -100 | Bb2       |
  | .ZZC | Our Pub  | c1   | c@    | member,ok,co,confirmed   | 01003 |     0 | Cc3       |
  And member is logged out

Scenario: A member clicks a Pay With Common Good button
  When member "?" visits page "pay-with-cg/company=NEWZZC&code=Cc3&item=food&amount=23.50"
  Then we show "Hello %PROJECT Member" with:
  | Pay        | $23.50 to Our Pub |
  | For        | food |
  | Account ID |  |

Scenario: A member submits a Pay With Common Good button payment with account ID
  When member "?" confirms form "pay-with-cg/company=NEWZZC&code=Cc3&item=food&amount=23" with values:
  | name   |*
  | NEWZZA |
  Then we say "status": "pay button success"
  And we message "new invoice" to member ".ZZA" with subs:
  | otherName | amount | purpose |*
  | Our Pub   | $23    | food    |
  And invoices:
  | nvid | created | status      | amount | payer | payee | for  |*
  |    1 | %today  | %TX_PENDING |     23 | .ZZA | .ZZC | food |

  When member "?" visits page "handle-invoice/nvid=1&code=TESTDOCODE"
  Then we show "Confirm Payment" with:
  | | Pay $23 to Our Pub for food. |
  | Pay | Dispute |

  When member "?" confirms form "handle-invoice/nvid=1&code=TESTDOCODE" with values:
  | op  |*
  | pay |
  Then we say "status": "You paid Our Pub $23."
  And invoices:
  | nvid | created | status | purpose |*
  |    1 | %today  |      1 | food    |
  And transactions:
  | xid | created | amount | payer | payee | for  |*
  |   1 | %today  |     23 | .ZZA  | .ZZC  | food |
  
  When member "?" visits page "handle-invoice/nvid=1&code=TESTDOCODE"
  Then we say "error": "inv already paid"

Scenario: A member clicks a Pay With Common Good button with variable amount
  When member "?" visits page "pay-with-cg/company=NEWZZC&code=Cc3&item=food&amount="
  Then we show "Hello %PROJECT Member" with:
  | Pay        | to Our Pub |
  | For        | food |
  | Account ID |  |

Scenario: A member submits a Pay With Common Good button payment with account ID and chosen amount
  When member "?" confirms form "pay-with-cg/company=NEWZZC&code=Cc3&item=food&amount=" with values:
  | name   | amount |*
  | NEWZZA |     23 |
  Then we say "status": "pay button success"
  And we message "new invoice" to member ".ZZA" with subs:
  | otherName | amount | purpose |*
  | Our Pub   | $23    | food    |
  And invoices:
  | nvid | created | status      | amount | payer | payee | for  |*
  |    1 | %today  | %TX_PENDING |     23 | .ZZA | .ZZC | food |

  When member "?" visits page "handle-invoice/nvid=1&code=TESTDOCODE"
  Then we show "Confirm Payment" with:
  | | Pay $23 to Our Pub for food. |
  | Pay | Dispute |

Scenario: A member clicks a button to buy store credit
  When member "?" visits page "pay-with-cg/company=NEWZZC&code=Cc3&for=credit&item=&amount=23.50"
  Then we show "Hello %PROJECT Member" with:
  | Pay        | $23.50 to Our Pub |
  | For        | store credit |
  | Account ID |  |

Scenario: A member types account ID to buy store credit
  When member "?" confirms form "pay-with-cg/company=NEWZZC&code=Cc3&for=credit&item=&amount=23" with values:
  | name   |*
  | NEWZZA |
  Then we say "status": "pay button success"
  And we message "new invoice" to member ".ZZA" with subs:
  | otherName | amount | purpose      |*
  | Our Pub   | $23    | store credit |
  And invoices:
  | nvid | created | status      | amount | payer | payee | for          |*
  |    1 | %today  | %TX_PENDING |     23 | .ZZA  | .ZZC  | store credit |

  When member "?" visits page "handle-invoice/nvid=1&code=TESTDOCODE"
  Then we show "Confirm Payment" with:
  | Pay $23 to Our Pub for store credit. |
  And with:
  | Pay | Dispute |

  When member "?" confirms form "handle-invoice/nvid=1&code=TESTDOCODE" with values:
  | op  |*
  | pay |
  Then we say "status": "You paid Our Pub $23."
  And invoices:
  | nvid | created | status | purpose      |*
  |    1 | %today  |      1 | store credit |
  And transactions:
  | xid | created | amount | payer | payee | for          |*
  |   1 | %today  |     23 | .ZZA  | .ZZC  | store credit |
  And these "tx_rules":
  | id | action     | payerType | payer | payeeType | payee | from         | to           | portion | amtMax |*
  |  1 | %ACT_SURTX | account   | .ZZA  | account   | .ZZC  | %MATCH_PAYEE | %MATCH_PAYER | .5      | 23     |
# after 6/1/2020  |  1 | %ACT_SURTX | account   | .ZZA  | account   | .ZZC  | %MATCH_PAYEE | %MATCH_PAYER | 1       | 23     |

Scenario: a member redeems store credit
  Given these "tx_rules":
  | id | action     | payerType | payer | payeeType | payee | from         | to           | portion | amtMax | end |*
  |  1 | %ACT_SURTX | account   | .ZZA  | account   | .ZZC  | %MATCH_PAYEE | %MATCH_PAYER | .50     | 23     |     |
  When member ".ZZA" confirms form "tx/pay" with values:
  | op  | who     | amount | goods      | purpose |*
  | pay | Our Pub | 20     | %FOR_GOODS | stuff   |
  Then transactions:
  | eid | xid | created | amount | payer | payee | purpose          | taking | rule | type      |*
  |   1 |   1 | %today  |     20 | .ZZA  | .ZZC | stuff             | 0      |      | %E_PRIME  |
  |   3 |   1 | %today  |     10 | .ZZC  | .ZZA | discount (rebate) | 0      | 1    | %E_REBATE |
  # MariaDb bug: autonumber skips id=2 when there are record ids 1 and -1

  When member ".ZZA" confirms form "tx/pay" with values:
  | op  | who     | amount | goods      | purpose |*
  | pay | Our Pub | 40     | %FOR_GOODS | stuff   |
  Then transactions:
  | eid | xid | created | amount | payer | payee | purpose          | taking | rule | type      |*
  |   4 |   2 | %today  |     40 | .ZZA  | .ZZC | stuff             | 0      |      | %E_PRIME  |
  |   5 |   2 | %today  |     13 | .ZZC  | .ZZA | discount (rebate) | 0      | 1    | %E_REBATE |
  And these "tx_rules":
  | id | end  |*
  |  1 | %now |

Scenario: A member clicks a button to buy a gift of store credit
  When member "?" visits page "pay-with-cg/company=NEWZZC&code=Cc3&for=gift&item=&amount=23.50"
  Then we show "Hello %PROJECT Member" with:
  | Pay          | $23.50 to Our Pub |
  | For          | store credit |
  | As a Gift to | |
  | Account ID   | |

Scenario: A member type account ID to buy a gift of store credit
  When member "?" confirms form "pay-with-cg/company=NEWZZC&code=Cc3&for=gift&item=&amount=23" with values:
  | for           | name          |*
  | b@example.com | a@example.com |
  Then we say "status": "pay button success"
  And we message "new invoice" to member ".ZZA" with subs:
  | otherName | amount | purpose                           |*
  | Our Pub   | $23    | gift of store credit (to Bea Two) |
  And invoices:
  | nvid | created | status      | amount | payer | payee | for                               |*
  |    1 | %today  | %TX_PENDING |     23 | .ZZA  | .ZZC  | gift of store credit (to Bea Two) |

  When member "?" visits page "handle-invoice/nvid=1&code=TESTDOCODE"
  Then we show "Confirm Payment" with:
  | Pay $23 to Our Pub for gift of store credit (to Bea Two) |
  And with:
  | Pay | Dispute |

  When member "?" confirms form "handle-invoice/nvid=1&code=TESTDOCODE" with values:
  | op  |*
  | pay |
  Then we say "status": "You paid Our Pub $23."
  And invoices:
  | nvid | created | status | purpose                           |*
  |    1 | %today  |      1 | gift of store credit (to Bea Two) |
  And transactions:
  | xid | created | amount | payer | payee | for                               |*
  |   1 | %today  |     23 | .ZZA  | .ZZC  | gift of store credit (to Bea Two) |
  And these "tx_rules":
  | id | payerType | payer | payeeType | payee | from         | to           | portion | amtMax |*
  |  1 | account   | .ZZB  | account   | .ZZC  | %MATCH_PAYEE | %MATCH_PAYER | 1       | 23     |

Scenario: A company gets an authcode
  When member "?" visits page "pay-with-cg/op=authcode&company=NEWZZC&cocode=Cc3"
  Then we exit showing:
  | cocode | now  | r | cry |*
  | Cc3    | %now | ? | 1   |

Scenario: A company has an api to process transaction results
  When member "?" confirms form "pay-with-cg/company=NEWZZC&code=Cc3&item=food&amount=23&return=http:%2F%2Fexample.com%2Fthanks.php&request=ABC123&api=http:%2F%2Fexample.com%2Fapi%2F" with values:
  | name   |*
  | NEWZZA |
  Then invoices:
  | nvid | created | status      | amount | payer | payee | for  |*
  |    1 | %today  | %TX_PENDING |     23 | .ZZA  | .ZZC  | food |
  And we redirect to "http://example.com/thanks.php?request=ABC123"

  When member "?" visits page "pay-with-cg/op=status&company=NEWZZC&cocode=Cc3&request=ABC123"
  Then we exit showing just "-1"
  
  When member "?" confirms form "handle-invoice/nvid=1&code=TESTDOCODE" with values:
  | op  |*
  | pay |
  Then transactions:
  | xid | created | amount | payer | payee | for  |*
  |   1 | %today  |     23 | .ZZA  | .ZZC  | food |
  And invoices:
  | nvid | created | status | amount | payer | payee | for  |*
  |    1 | %today  | 1      |     23 | .ZZA  | .ZZC  | food |
  And we hit "http://example.com/api/" with:
  | request | ok | msg                   |*
  | ABC123  | 1  | You paid Our Pub $23. |

  When member "?" visits page "pay-with-cg/op=status&company=NEWZZC&cocode=Cc3&request=ABC123"
  Then we exit showing just "1"
