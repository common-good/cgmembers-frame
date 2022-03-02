Feature: Invoice
AS a member
I WANT to charge other members and pay invoices from other members
SO I can buy and sell stuff.

Setup:
  Given members:
  | uid  | fullName | floor | flags             | risks   | bankAccount  |*
  | .ZZA | Abe One  |     0 | ok,confirmed      |         |              |
  | .ZZB | Bea Two  |  -250 | ok,confirmed,debt |         |              |
  | .ZZC | Our Pub  |     0 | ok,confirmed,co   | hasBank | %T_BANK_ACCT |
  | .ZZD | Dee Four |     0 |                   |         |              |
  And these "u_relations":
  | main | agent | permission |*
  | .ZZC | .ZZB  | buy        |
  Then balances:
  | uid  | balance |*
  | .ZZA |       0 |
  | .ZZB |       0 |
  | .ZZC |       0 |

Scenario: A member confirms request to charge another member
  When member ".ZZA" confirms form "tx/charge" with values:
  | op     | who     | amount | goods      | purpose |*
  | charge | Bea Two | 100    | %FOR_GOODS | labor   |
  Then these "tx_requests":
  | nvid | created | status      | amount | payer | payee | for   |*
  |    1 | %today  | %TX_PENDING |    100 | .ZZB | .ZZA | labor |
  And we message "invoiced you" to member ".ZZB" with subs:
  | otherName | amount | purpose |*
  | Abe One   | $100   | labor   |

  When member ".ZZB" visits page "handle-invoice/nvid=1&code=TESTDOCODE"
  Then we show "Confirm Payment" with:
  | ~question | Pay $100 to Abe One for labor |
  | Amount to Pay | 100 |
  | ~ | Pay |
  | Reason ||
  | ~ | Dispute |

  When member ".ZZB" confirms form "handle-invoice/nvid=1&code=TESTDOCODE" with values:
  | op   | ret | nvid | payAmount | payer | payee | purpose | created |*
  | pay  |     |    1 |       100 | .ZZB  | .ZZA  | labor   | %today  |
  Then these "txs":
  | xid | created | amount | payer | payee | purpose | taking | relType | rel |*
  |   1 | %today  |    100 | .ZZB | .ZZA | labor     | 0      | I       | 1   |
  And these "tx_requests":
  | nvid | created | status | amount | payer | payee | for   |*
  |    1 | %today  | 1      |    100 | .ZZB | .ZZA | labor |
  And balances:
  | uid  | balance |*
  | .ZZA |     100 |
  | .ZZB |    -100 |
  | .ZZC |       0 |
  
Scenario: A member makes partial payments
  Given these "tx_requests":
  | nvid | created | status      | amount | payer | payee | for   |*
  |    1 | %today  | %TX_PENDING |    100 | .ZZB | .ZZA | labor |
  When member ".ZZB" confirms form "handle-invoice/nvid=1&code=TESTDOCODE" with values:
  | op   | ret | nvid | payAmount | payer | payee | purpose | created |*
  | pay  |     |    1 |        10 | .ZZB  | .ZZA  | labor   | %today  |
  Then we say "status": "report tx|left on invoice" with subs:
  | did    | otherName | amount | remaining |*
  | paid   | Abe One   | $10    | $90       |
  And these "txs":
  | xid | created | amount | payer | payee | purpose | taking | relType | rel |*
  |   1 | %today  |     10 | .ZZB  | .ZZA  | labor   | 0      | I       | 1   |
  And these "tx_requests":
  | nvid | created | status      | amount | payer | payee | for   |*
  |    1 | %today  | %TX_PENDING |    100 | .ZZB  | .ZZA  | labor |
  
  When member ".ZZB" visits page "handle-invoice/nvid=1&code=TESTDOCODE"
  Then we show "Confirm Payment" with:
  | ~question | Pay $100 to Abe One for labor |
  | Amount to Pay | 90 |
  | ~ | Pay |
  | Reason ||
  | ~ | Dispute |

  When member ".ZZB" confirms form "handle-invoice/nvid=1&code=TESTDOCODE" with values:
  | op   | ret | nvid | payAmount | payer | payee | purpose | created |*
  | pay  |     |    1 |        90 | .ZZB  | .ZZA  | labor   | %today  |
  Then we say "status": "report tx" with subs:
  | did    | otherName | amount |*
  | paid   | Abe One   | $90    |
  And these "txs":
  | xid | created | amount | payer | payee | purpose | taking | relType | rel |*
  |   2 | %today  |     90 | .ZZB  | .ZZA  | labor   | 0      | I       | 1   |
  And these "tx_requests":
  | nvid | created | status | amount | payer | payee | for   |*
  |    1 | %today  | 2      |    100 | .ZZB  | .ZZA  | labor |
  
  When member ".ZZB" visits page "history/transactions/period=15"
  Then we show "Transaction History" with:
  | Tx# | Date | Name    | Purpose                  | Amount |  Balance |
  |  2  | %mdy | Abe One | labor (CG inv#1 final)   | 90.00  |  -100.00 |
  |  1  | %mdy | Abe One | labor (CG inv#1 partial) | 10.00  |   -10.00 |

Scenario: A member confirms request to charge another member who has a bank account
  When member ".ZZA" confirms form "tx/charge" with values:
  | op     | who     | amount | goods      | purpose |*
  | charge | Our Pub | 100    | %FOR_GOODS | stuff   |
  Then these "tx_requests":
  | nvid | created | status      | amount | payer | payee | for   |*
  |    1 | %today  | %TX_PENDING |    100 | .ZZC | .ZZA | stuff |
  And we message "invoiced you" to member ".ZZC" with subs:
  | otherName | amount | purpose |*
  | Abe One   | $100   | stuff   |

Scenario: A member confirms request to charge a not-yet member
  When member ".ZZA" confirms form "tx/charge" with values:
  | op     | who      | amount | goods          | purpose |*
  | charge | Dee Four | 100    | %FOR_GOODS     | labor   |
  Then these "tx_requests":
  | nvid | created | status      | amount | payer | payee | for   |*
  |    1 | %today  | %TX_PENDING |    100 | .ZZD | .ZZA | labor |
  And we message "invoiced you" to member ".ZZD" with subs:
  | otherName | amount | purpose |*
  | Abe One   | $100   | labor   |

  When member ".ZZD" visits page "handle-invoice/nvid=1"
  Then we show "Confirm Payment" with:
  | ~question | Pay $100 to Abe One for labor |
  | ~ | Pay |
  | Reason ||
  | ~ | Dispute |

  When member ".ZZD" confirms form "handle-invoice/nvid=1" with values:
  | op   | ret | nvid | payAmount | payer | payee | purpose | created |*
  | pay  |     |    1 |       100 | .ZZD  | .ZZA  | labor   | %today  |
  Then these "tx_requests":
  | nvid | created | status       | amount | payer | payee | for   |*
  |    1 | %today  | %TX_APPROVED |    100 | .ZZD | .ZZA | labor |
  And we say "status": "finish signup|when funded"
  
Scenario: A member denies an invoice
  When member ".ZZA" confirms form "tx/charge" with values:
  | op     | who     | amount | goods | purpose |*
  | charge | Bea Two | 100    | %FOR_GOODS     | labor   |
  And member ".ZZB" confirms form "handle-invoice/nvid=1" with values:
  | op   | ret | nvid | payAmount | payer | payee | purpose | created | whyNot |*
  | deny |     |    1 |       100 | .ZZB  | .ZZA  | labor   | %today  | broke  |
  Then these "tx_requests":
  | nvid | created | status     | amount | payer | payee | for   |*
  |    1 | %today  | %TX_DENIED |    100 | .ZZB | .ZZA | labor |
  And we message "invoice denied" to member ".ZZA" with subs:
  | payerName | created | amount | purpose | reason |*
  | Bea Two   | %mdY    |   $100 | labor   | broke  |
  And balances:
  | uid  | balance |*
  | .ZZA |       0 |
  | .ZZB |       0 |
  | .ZZC |       0 |

Scenario: A member approves an invoice with insufficient funds
  When member ".ZZA" confirms form "tx/charge" with values:
  | op     | who     | amount | goods | purpose |*
  | charge | Bea Two | 300    | %FOR_GOODS     | labor   |
  And member ".ZZB" confirms form "handle-invoice/nvid=1" with values:
  | op   | ret | nvid | payAmount | payer | payee | purpose | created | whyNot |*
  | pay  |     |    1 |       300 | .ZZB  | .ZZA  | labor   | %today  |        |
  Then these "tx_requests":
  | nvid | created | status       | amount | payer | payee | for   |*
  |    1 | %today  | %TX_APPROVED |    300 | .ZZB | .ZZA | labor |
  And we say "error": "short invoice|when funded|how to fund" with subs:
  | short | payeeName | nvid |*
  | $50   | Abe One   |    1 |
  And balances:
  | uid  | balance |*
  | .ZZA |       0 |
  | .ZZB |       0 |
  | .ZZC |       0 |

Scenario: A member approves invoices forevermore
  Given members have:
  | uid  | risks   |*
  | .ZZB | hasBank |
  When member ".ZZA" confirms form "tx/charge" with values:
  | op     | who     | amount | goods      | purpose |*
  | charge | Bea Two | 300    | %FOR_GOODS | labor   |
  And member ".ZZB" confirms form "handle-invoice/nvid=1" with values:
  | op   | ret | nvid | payAmount | payer | payee | purpose | created | whyNot | always |*
  | pay  |     |    1 |       300 | .ZZB  | .ZZA  | labor   | %today  |        |      1 |
  Then these "tx_requests":
  | nvid | created | status       | amount | payer | payee | for   |*
  |    1 | %today  | %TX_APPROVED |    300 | .ZZB | .ZZA | labor |
  And these "u_relations":
  | main | agent | flags            |*
  | .ZZA | .ZZB  | customer,autopay |

Scenario: A member approves an invoice to a trusting customer
  Given these "u_relations":
  | main | agent | flags            |*
  | .ZZA | .ZZB  | customer,autopay |
  When member ".ZZA" confirms form "tx/charge" with values:
  | op     | who     | amount | goods      | purpose |*
  | charge | Bea Two | 100    | %FOR_GOODS | labor   |
  Then these "txs":
  | xid | created | amount | payer | payee | purpose | taking | relType | rel |*
  |   1 | %today  |    100 | .ZZB | .ZZA | labor     | 0      | I       | 1   |
  And these "tx_requests":
  | nvid | created | status | amount | payer | payee | for   |*
  |    1 | %today  | 1      |    100 | .ZZB | .ZZA | labor |
  And balances:
  | uid  | balance |*
  | .ZZA |     100 |
  | .ZZB |    -100 |
  | .ZZC |       0 |
  