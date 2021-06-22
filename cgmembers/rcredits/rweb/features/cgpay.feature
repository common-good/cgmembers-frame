Feature: A user clicks a "CGPay" button on a participating company's website
AS a member
I WANT to pay a member company or individual by clicking a CGPay button
SO I can get stuff, buy credit, or make donations easily.

Setup:
  Given members:
  | uid  | fullName | pass | email | flags                    | zip   | floor | emailCode |*
  | .ZZA | Abe One  | a1   | a@    | member,ok,confirmed,debt | 01001 |  -100 | Aa1       |
  | .ZZB | Bea Two  | b1   | b@    | member,ok,confirmed,debt | 01001 |  -100 | Bb2       |
  | .ZZC | Our Pub  | c1   | c@    | member,ok,co,confirmed   | 01003 |     0 | Cc3       |
  And relations:
  | main | other | permission |*
  | .ZZC | .ZZB  | manage     |
  And member is logged out

Scenario: A member clicks a CGPay button
  Given a button code for:
  | account | secret | item | amount |*
  | .ZZC    | Cc3    | food | 23.50  |
  When member "?" visits page "cgpay?code=TESTCODE"
  Then we show "Pay with %PROJECT" with:
  | Pay        | 23.50 to Our Pub |
  | For        | food |
  | Account ID |  |
  | Password   |  |

Scenario: A member clicks an expired CGPay button
  Given a button code for:
  | account | secret | item | amount | expires |*
  | .ZZC    | Cc3    | food | 23.50  | %now-1d |
  When member "?" visits page "cgpay?code=TESTCODE"
  Then we say "error": "button expired"

Scenario: A member submits a CGPay button payment with account ID
  Given a button code for:
  | account | secret | item | amount |*
  | .ZZC    | Cc3    | food | 23     |
  When member "?" confirms "cgpay?code=TESTCODE" with:
  | qid  | pass |*
  | .ZZA | a1   |
  Then we say "status": "success title|report tx" with subs:
  | did  | otherName | amount |*
  | paid | Our Pub   | $23    |
  And transactions:
  | xid | created | amount | payer | payee | for  |*
  |   1 | %today  |     23 | .ZZA  | .ZZC  | food |

Scenario: A member clicks a CGPay button with variable amount
  Given a button code for:
  | account | secret | item |*
  | .ZZC    | Cc3    | food |
  When member "?" visits page "cgpay?code=TESTCODE"
  Then we show "Pay with %PROJECT" with:
  | Pay        | to Our Pub |
  | For        | food |
  | Account ID |  |
  | Password   |  |

Scenario: A member submits a CGPay button payment with account ID and chosen amount
  Given a button code for:
  | account | secret | item |*
  | .ZZC    | Cc3    | food |
  When member "?" confirms "cgpay?code=TESTCODE" with:
  | qid  | amount | pass |*
  | .ZZA | $23.45 | a1   |
  Then we say "status": "success title|report tx" with subs:
  | did  | otherName | amount |*
  | paid | Our Pub   | $23.45 |

Scenario: A member clicks a button to buy 50% store credit
  Given a button code for:
  | account | secret | for      | amount |*
  | .ZZC    | Cc3    | credit50 | 23.50  |
  When member "?" visits page "cgpay?code=TESTCODE"
  Then we show "Pay with %PROJECT" with:
  | Pay        | 23.50 to Our Pub |
  | For        | store credit |
  | Account ID |  |
  | Password   |  |
  
Scenario: A member clicks a button to buy store credit for a different amount
  Given a button code for:
  | account | secret | for    | amount | credit |*
  | .ZZC    | Cc3    | credit | 23     | 30     |
  When member "?" visits page "cgpay?code=TESTCODE"
  Then we show "Pay with %PROJECT" with:
  | Pay        | 23.00 to Our Pub |
  | For        | $30 store credit |
  | Account ID |  |
  | Password   |  |
  
  When member "?" confirms "cgpay?code=TESTCODE" with:
  | qid  | pass |*
  | .ZZA | a1   |
  Then we say "status": "success title|report tx" with subs:
  | did  | otherName | amount |*
  | paid | Our Pub   | $23    |
  And transactions:
  | xid | created | amount | payer | payee | for              | rule |*
  |   1 | %today  |     23 | .ZZA  | .ZZC  | $30 store credit |    1 |
  And these "tx_rules":
  | id | action     | payerType | payer | payeeType | payee | from         | to           | portion | amtMax |*
  |  1 | %ACT_SURTX | account   | .ZZA  | account   | .ZZC  | %MATCH_PAYEE | %MATCH_PAYER | 1       | 30     |

Scenario: A member cancels their purchase of store credit
  Given transactions:
  | xid | created | amount | payer | payee | for              | rule |*
  |   1 | %today  |     23 | .ZZA  | .ZZC  | $30 store credit |    1 |
  And these "tx_rules":
  | id | action     | payerType | payer | payeeType | payee | from         | to           | portion | amtMax |*
  |  1 | %ACT_SURTX | account   | .ZZA  | account   | .ZZC  | %MATCH_PAYEE | %MATCH_PAYER | 1       | 30     |
  When member "C:B" visits page "history/transactions/period=5"
  And member "C:B" clicks "X" on transaction 1
  Then transactions:
  | xid | created | amount | payer | payee | for               | rule |*
  |   2 | %today  |    -23 | .ZZA  | .ZZC  | $30 store credit  |    1 |
  And these "tx_rules":
  | id | end  |*
  |  1 | %now |
  And we notice "your credit canceled" to member ".ZZA" with subs:
  | amount | co      |*
  | $23    | Our Pub |
  And we notice "customer credit canceled" to member ".ZZC" with subs:
  | amount | customer |*
  | $23    | Abe One  |

Scenario: A member types account ID to buy 50% store credit
  Given a button code for:
  | account | secret | for      | amount |*
  | .ZZC    | Cc3    | credit50 | 23     |
  When member "?" confirms "cgpay?code=TESTCODE" with:
  | qid  | pass |*
  | .ZZA | a1   |
  Then we say "status": "success title|report tx" with subs:
  | did  | otherName | amount |*
  | paid | Our Pub   | $23    |
  And transactions:
  | xid | created | amount | payer | payee | for          |*
  |   1 | %today  |     23 | .ZZA  | .ZZC  | store credit |
  And these "tx_rules":
  | id | action     | payerType | payer | payeeType | payee | from         | to           | portion | amtMax |*
  |  1 | %ACT_SURTX | account   | .ZZA  | account   | .ZZC  | %MATCH_PAYEE | %MATCH_PAYER | .5      | 23     |

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
  # MariaDb bug: autonumber passes over id=2 when there are record ids 1 and -1

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
  Given a button code for:
  | account | secret | for    | amount | for  |*
  | .ZZC    | Cc3    | credit | 23.50  | gift |
  When member "?" visits page "cgpay?code=TESTCODE"
  Then we show "Pay with %PROJECT" with:
  | Pay          | 23.50 to Our Pub |
  | For          | store credit |
  | As a Gift to | |
  | Account ID   | |
  | Password     | |

Scenario: A member types account ID to buy a gift of store credit
  Given a button code for:
  | account | secret | for      | amount | for  |*
  | .ZZC    | Cc3    | credit50 | 23     | gift |
  When member "?" confirms "cgpay?code=TESTCODE" with:
  | for           | qid           | pass |*
  | b@example.com | a@example.com | a1   |
  Then we say "status": "success title|report tx" with subs:
  | did  | otherName | amount |*
  | paid | Our Pub   | $23    |
  And transactions:
  | xid | created | amount | payer | payee | for                               |*
  |   1 | %today  |     23 | .ZZA  | .ZZC  | gift of store credit (to Bea Two) |
  And these "tx_rules":
  | id | payerType | payer | payeeType | payee | from         | to           | portion | amtMax |*
  |  1 | account   | .ZZB  | account   | .ZZC  | %MATCH_PAYEE | %MATCH_PAYER | 1       | 23     |
