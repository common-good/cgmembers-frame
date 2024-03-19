Feature: Invoice
AS a member
I WANT to charge other members and pay invoices from other members
SO I can buy and sell stuff.

Setup:
  Given members:
  | uid  | fullName | floor | flags             | risks   | bankAccount  | phone        |*
  | .ZZA | Abe One  |   -10 | ok,confirmed      |         |              | +14136280001 |
  | .ZZB | Bea Two  |  -250 | ok,confirmed,debt |         |              | +14136280002 |
  | .ZZC | Our Pub  |   -30 | ok,confirmed,co   | hasBank | %T_BANK_ACCT | +14136280003 |
  | .ZZD | Dee Four |     0 | debt              |         |              | +14136280004 |
  And these "u_relations":
  | main | agent | permission |*
  | .ZZC | .ZZB  | buy        |
  Then balances:
  | uid  | balance |*
  | .ZZA |       0 |
  | .ZZB |       0 |
  | .ZZC |       0 |

Scenario: A member approves an invoice
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
  | ~question     | Pay $100 to Abe One for labor |
  | Amount to Pay | 100 |
  | ~             | Pay |
  | Reason        |     |
  | ~             | Dispute |

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

Scenario: A member overpays an invoice
  Given these "tx_requests":
  | nvid | created | status      | amount | payer | payee | for   |*
  |    1 | %today  | %TX_PENDING |    100 | .ZZB | .ZZA | labor |
  When member ".ZZB" confirms form "handle-invoice/nvid=1&code=TESTDOCODE" with values:
  | op   | ret | nvid | payAmount | payer | payee | purpose | created |*
  | pay  |     |    1 |       110 | .ZZB  | .ZZA  | labor   | %today  |
  Then we say "status": "report tx|left on invoice" with subs:
  | did    | otherName | amount | remaining |*
  | paid   | Abe One   | $110   | $-10      |
  And these "txs":
  | xid | created | amount | payer | payee | purpose | taking | relType | rel |*
  |   1 | %today  |    110 | .ZZB  | .ZZA  | labor   | 0      | I       | 1   |
  And these "tx_requests":
  | nvid | created | status      | amount | payer | payee | for   |*
  |    1 | %today  | 1           |    100 | .ZZB  | .ZZA  | labor |
  
  When member ".ZZB" visits page "history/transactions/period=15"
  Then we show "Transaction History" with:
  | Tx# | Date | Name    | Purpose                     | Amount | Balance |
  |  1  | %mdy | Abe One | labor (CG inv#1 - overpaid) | 110.00 | -110.00 |
  
Scenario: A member who has a bank account approves an invoice
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
  | otherName | amount | purpose | balance | creditLine                          |*
  | Abe One   | $100   | labor   |      $0 | $0 (based on your monthly activity) |

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
  | op   | ret | nvid | payAmount | payer | payee | purpose | created | reason |*
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
  | op   | ret | nvid | payAmount | payer | payee | purpose | created | reason |*
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
  | op   | ret | nvid | payAmount | payer | payee | purpose | created | reason | always |*
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
  
Scenario: A payee marks an invoice paid manually
  Given these "tx_requests":
  | nvid | created | status      | amount | payer | payee | for   |*
  |    1 | %today  | %TX_PENDING |    300 | .ZZB  | .ZZA  | labor |
  When member ".ZZA" visits "handle-invoice/nvid=1"
  Then we show "Unpaid Invoice" with:
  | Handle invoice #1 (%mdY) charging Bea Two $300 for labor ||
  | Action ||
  | mark paid | cancel |
  | Reason ||
  || Close |
  
  When member ".ZZA" confirms form "handle-invoice/nvid=1" with values:
  | op    | ret | nvid | payAmount | payer | payee | purpose | created | reason | action |*
  | close |     |    1 |       300 | .ZZB  | .ZZA  | labor   | %today  | cuz    |      0 |
  Then these "tx_requests":
  | nvid | created | status   | amount | payer | payee | for   | reason |*
  |    1 | %today  | %TX_PAID |    300 | .ZZB  | .ZZA  | labor | cuz    |
  And we message "invoice withdrawn" to member ".ZZB" with subs:
  | amount | payerName | payeeName | created | purpose | reason | done        |*
  | $300   | Bea Two   | Abe One   | %mdY    | labor   | cuz    | marked PAID |

Scenario: A payee cancels an invoice
  Given these "tx_requests":
  | nvid | created | status      | amount | payer | payee | for   |*
  |    1 | %today  | %TX_PENDING |    300 | .ZZB  | .ZZA  | labor |
  When member ".ZZA" confirms form "handle-invoice/nvid=1" with values:
  | op    | ret | nvid | payAmount | payer | payee | purpose | created | reason | action |*
  | close |     |    1 |       300 | .ZZB  | .ZZA  | labor   | %today  | cuz    |      1 |
  Then these "tx_requests":
  | nvid | created | status       | amount | payer | payee | for   | reason |*
  |    1 | %today  | %TX_CANCELED |    300 | .ZZB  | .ZZA  | labor | cuz    |
  And we message "invoice withdrawn" to member ".ZZB" with subs:
  | amount | payerName | payeeName | created | purpose | reason | done        |*
  | $300   | Bea Two   | Abe One   | %mdY    | labor   | cuz    | canceled    |

Scenario: A payee reopens an invoice
  Given these "tx_requests":
  | nvid | created | status   | amount | payer | payee | for   | reason |*
  |    1 | %today  | %TX_PAID |    300 | .ZZB  | .ZZA  | labor | cuz    |
  When member ".ZZA" visits "handle-invoice/nvid=1"
  Then we show "Invoice Was Closed Manually" with:
  | Reopen invoice #1 (%mdY) charging Bea Two $300 for labor? | |
  | Closed Because | cuz    |
  |                | Reopen |
  When member ".ZZA" confirms form "handle-invoice/nvid=1" with values:
  | op     | ret | nvid | payAmount | payer | payee | purpose | created |*
  | reopen |     |    1 |       300 | .ZZB  | .ZZA  | labor   | %today  |
  Then these "tx_requests":
  | nvid | created | status       | amount | payer | payee | for   | reason |*
  |    1 | %today  | %TX_PENDING  |    300 | .ZZB  | .ZZA  | labor | cuz    |
  And we message "invoice reopened|invoiced you" to member ".ZZB" with subs:
  | amount | otherName | otherEmail    | otherPhone      | payerName | payeeName | created | purpose |*
  | $300   | Abe One   | a@example.com | +1 413 628 0001 | Bea Two   | Abe One   |%mdY     | labor   |
