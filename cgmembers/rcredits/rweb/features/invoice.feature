Feature: Invoice
AS a member
I WANT to charge other members and pay invoices from other members
SO I can buy and sell stuff.

Setup:
  Given members:
  | uid  | fullName | floor | flags             | risks   | bankAccount  | phone        |*
  | .ZZA | Abe One  |   -10 | ok,confirmed      |         |              | +14136280001 |
  | .ZZB | Bea Two  |  -250 | ok,confirmed,debt | hasBank | %T_BANK_ACCT | +14136280002 |
  | .ZZC | Our Pub  |   -30 | ok,confirmed,co   | hasBank | %T_BANK_ACCT | +14136280003 |
  | .ZZD | Dee Four |     0 | debt              |         |              | +14136280004 |
  And these "u_relations":
  | main | agent | permission |*
  | .ZZC | .ZZB  | buy        |
  And var "code" encrypts:
  | op  | v |*
  | inv | 1 |
  Then balances:
  | uid  | balance |*
  | .ZZA |       0 |
  | .ZZB |       0 |
  | .ZZC |       0 |

Scenario: A member invoices someone
  When member ".ZZA" confirms form "tx/charge" with values:
  | op     | who     | amount | goods      | purpose |*
  | charge | Bea Two | 100    | %FOR_GOODS | labor   |
  Then these "tx_requests":
  | nvid | created | status  | amount | payer | payee | for   |*
  |    1 | %today  | open    |    100 | .ZZB  | .ZZA  | labor |
  And we message "invoiced you" to member ".ZZB" with subs:
  | otherName | amount | purpose |*
  | Abe One   | $100   | labor   |

Scenario: A member receives an invoice with a non-positive balance
  Given these "tx_requests":
  | nvid | created | status  | amount | payer | payee | for   |*
  |    1 | %today  | open    |    100 | .ZZB  | .ZZA  | labor |
  And balances:
  | uid  | balance |*
  | .ZZB |       0 |
  When member ".ZZB" visits page "handle-invoice/nvid=1&code=%code"
  Then we show "Confirm Payment" with:
  | ~question     | Pay $100 to Abe One for labor |
  | Amount to Pay | 100     |
  | Pay           | dispute |
  | Reason        |         |
  | Dispute       |         |

Scenario: A member receives an invoice with no connected bank account
  Given these "tx_requests":
  | nvid | created | status  | amount | payer | payee | for   |*
  |    1 | %today  | open    |    100 | .ZZB  | .ZZA  | labor |
  And members have:
  | uid  | balance | risks | bankAccount |*
  | .ZZB |      10 |       |             |
  When member ".ZZB" visits page "handle-invoice/nvid=1&code=%code"
  Then we show "Confirm Payment" with:
  | ~question     | Pay $100 to Abe One for labor |
  | Amount to Pay | 100     |
  | Pay           | dispute |
  | Reason        |         |
  | Dispute       |         |

Scenario: A member receives an invoice with a positive balance
  Given these "tx_requests":
  | nvid | created | status  | amount | payer | payee | for   |*
  |    1 | %today  | open    |    100 | .ZZB  | .ZZA  | labor |
  And balances:
  | uid  | balance |*
  | .ZZB |      10 |
  When member ".ZZB" visits page "handle-invoice/nvid=1&code=%code"
  Then we show "Confirm Payment" with:
  | ~question     | Pay $100 to Abe One for labor |
  | Amount to Pay | 100 |
  | | Pull this entire amount from your bank account. |
  | | Pay first from your Common Good balance; then from your bank account as needed. |
  | Pay           | dispute |
  | Reason        |         |
  | Dispute       |         |
  And without:
  | Category |

Scenario: A fiscally sponsored partner receives an invoice
  Given members have:
  | uid  | coFlags   |*
  | .ZZC | sponsored |
  And these "tx_requests":
  | nvid | created | status  | amount | payer | payee | for   |*
  |    1 | %today  | open    |    100 | .ZZC  | .ZZA  | labor |
  And balances:
  | uid  | balance |*
  | .ZZC |     100 |
  When member ".ZZC" visits page "handle-invoice/nvid=1&code=%code"
  Then we show "Confirm Payment" with:
  | ~question     | Pay $100 to Abe One for labor |
  | Amount to Pay | 100 |
  | Category      |     |
  | | Pull this entire amount from your bank account. |
  | | Pay first from your Common Good balance; then from your bank account as needed. |
  | Pay           | dispute |
  | Reason        |         |
  | Dispute       |         |

Scenario: A member pays an invoice with sufficient balance
  Given balances:
  | uid  | balance |*
  | .ZZB | 300     |
  And member ".ZZA" confirms form "tx/charge" with values:
  | op     | who     | amount | goods      | purpose |*
  | charge | Bea Two | 100    | %FOR_GOODS | labor   |
  When member ".ZZB" confirms form "handle-invoice/nvid=1&code=%code" with values:
  | op   | ret | nvid | payAmount | payer | payee | purpose | created | balFirst |*
  | pay  |     |    1 |       100 | .ZZB  | .ZZA  | labor   | %today  | 1    |
  Then these "txs":
  | xid | created | amount | payer | payee | purpose | taking | relType | rel |*
  |   1 | %today  |    100 | .ZZB  | .ZZA  | labor     | 0      | I       | 1   |
  And balances:
  | uid  | balance |*
  | .ZZA |     100 |
  | .ZZB |     200 |
  | .ZZC |       0 |

Scenario: A fiscally sponsored partner pays an invoice with sufficient balance
  Given members have:
  | uid  | balance | coFlags   |*
  | .ZZC | 300     | sponsored |
  And member ".ZZA" confirms form "tx/charge" with values:
  | op     | who     | amount | goods      | purpose |*
  | charge | Our Pub | 100    | %FOR_GOODS | labor   |
  When member ".ZZC" confirms form "handle-invoice/nvid=1&code=%code" with values:
  | op   | ret | nvid | payAmount | cat          | payer | payee | purpose | created | balFirst |*
  | pay  |     |    1 |       100 | GRANT-PERSON | .ZZC  | .ZZA  | labor   | %today  | 1    |
  Then these "txs":
  | xid | created | amount | payer | payee | purpose | cat1         | taking | relType | rel |*
  |   1 | %today  |    100 | .ZZC  | .ZZA  | labor   | GRANT-PERSON | 0      | I       | 1   |
  And balances:
  | uid  | balance |*
  | .ZZA |     100 |
  | .ZZB |       0 |
  | .ZZC |     200 |
  
Scenario: A member makes partial payments with sufficient balance
  Given these "tx_requests":
  | nvid | created | status  | amount | payer | payee | for   |*
  |    1 | %today  | open    |    100 | .ZZB  | .ZZA  | labor |
  And balances:
  | uid  | balance |*
  | .ZZB | 100     |
  When member ".ZZB" confirms form "handle-invoice/nvid=1&code=%code" with values:
  | op   | ret | nvid | payAmount | payer | payee | purpose | created | balFirst |*
  | pay  |     |    1 |        10 | .ZZB  | .ZZA  | labor   | %today  | 1    |
  Then these "tx_requests":
  | nvid | created | status   | amount | payer | payee | for   |*
  |    1 | %today  | approved |    100 | .ZZB  | .ZZA  | labor |
  And we say "status": "report tx|left on invoice" with subs:
  | did    | otherName | amount | remaining |*
  | paid   | Abe One   | $10    | $90       |
  And these "txs":
  | xid | created | amount | payer | payee | purpose | taking | relType | rel |*
  |   1 | %today  |     10 | .ZZB  | .ZZA  | labor   | 0      | I       | 1   |
  
  When member ".ZZB" visits page "handle-invoice/nvid=1&code=%code"
  Then we show "Confirm Payment" with:
  | ~question | Pay $100 to Abe One for labor |
  | Amount to Pay | 90 |
  | ~ | Pay |
  | Reason ||
  | ~ | dispute |

  When member ".ZZB" confirms form "handle-invoice/nvid=1&code=%code" with values:
  | op   | ret | nvid | payAmount | payer | payee | purpose | created | balFirst |*
  | pay  |     |    1 |        90 | .ZZB  | .ZZA  | labor   | %today  | 0    |
  Then we say "status": "report tx|expect a transfer" with subs:
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
  |  3  | %mdy |         | from bank                |  90.00 |    90.00 |
  |  2  | %mdy | Abe One | labor (CG inv#1 final)   | -90.00 |     0.00 |
  |  1  | %mdy | Abe One | labor (CG inv#1 partial) | -10.00 |    90.00 |

Scenario: A member makes partial payments with insufficient balance
  Given these "tx_requests":
  | nvid | created | status  | amount | payer | payee | for   |*
  |    1 | %today  | open    |    100 | .ZZB  | .ZZA  | labor |
  When member ".ZZB" confirms form "handle-invoice/nvid=1&code=%code" with values:
  | op   | ret | nvid | payAmount | payer | payee | purpose | created | balFirst |*
  | pay  |     |    1 |        10 | .ZZB  | .ZZA  | labor   | %today  | 1    |
  Then these "tx_requests":
  | nvid | created | status   | amount | payer | payee | for   |*
  |    1 | %today  | approved |    100 | .ZZB  | .ZZA  | labor |
  And we say "status": "report tx|left on invoice|expect a transfer" with subs:
  | did    | otherName | amount | remaining |*
  | paid   | Abe One   | $10    | $90       |
  And these "txs":
  | xid | created | amount | payer | payee | purpose   | taking | relType | rel |*
  |   1 | %today  |     10 | .ZZB  | .ZZA  | labor     | 0      | I       | 1   |
  And these "txs":
  | xid | created | amount | payer | payee | purpose   | taking |*
  |   2 | %today  |     10 | bank  | .ZZB  | from bank | 0      |
  And these "txs2":
  | txid | created | completed | amount | payee |*
  |    1 | %now    | %now      |     10 | .ZZB  |

  When member ".ZZB" confirms form "handle-invoice/nvid=1&code=%code" with values:
  | op   | ret | nvid | payAmount | payer | payee | purpose | created | balFirst |*
  | pay  |     |    1 |        90 | .ZZB  | .ZZA  | labor   | %today  | 0    |
  Then we say "status": "report tx|expect a transfer" with subs:
  | did    | otherName | amount |*
  | paid   | Abe One   | $90    |
  And these "txs":
  | xid | created | amount | payer | payee | purpose | taking | relType | rel |*
  |   3 | %today  |     90 | .ZZB  | .ZZA  | labor   | 0      | I       | 1   |
  And these "txs":
  | xid | created | amount | payer | payee | purpose   | taking |*
  |   4 | %today  |     90 | bank  | .ZZB  | from bank | 0      |
  And these "tx_requests":
  | nvid | created | status | amount | payer | payee | for   |*
  |    1 | %today  | 3      |    100 | .ZZB  | .ZZA  | labor |
  And these "txs2":
  | txid | created | completed | amount | payee |*
  |    2 | %now    | %now      |     90 | .ZZB  |
  
  When member ".ZZB" visits page "history/transactions/period=15"
  Then we show "Transaction History" with:
  | Tx# | Date | Name    | Purpose                  | Amount |  Balance |
  |  4  | %mdy |         | from bank                |  90.00 |     0.00 |
  |  3  | %mdy | Abe One | labor (CG inv#1 final)   | -90.00 |   -90.00 |
  |  2  | %mdy |         | from bank                |  10.00 |     0.00 |
  |  1  | %mdy | Abe One | labor (CG inv#1 partial) | -10.00 |   -10.00 |

Scenario: A member overpays an invoice
  Given these "tx_requests":
  | nvid | created | status  | amount | payer | payee | for   |*
  |    1 | %today  | open    |    100 | .ZZB  | .ZZA  | labor |
  When member ".ZZB" confirms form "handle-invoice/nvid=1&code=%code" with values:
  | op   | ret | nvid | payAmount | payer | payee | purpose | created | balFirst |*
  | pay  |     |    1 |       110 | .ZZB  | .ZZA  | labor   | %today  | 1    |
  Then we say "error": "amount too big" with subs:
  | max | 100.00 |**
#  Then we say "status": "report tx|left on invoice" with subs:
#  | did    | otherName | amount | remaining |*
#  | paid   | Abe One   | $110   | $-10      |
#  And these "txs":
#  | xid | created | amount | payer | payee | purpose | taking | relType | rel |*
#  |   1 | %today  |    110 | .ZZB  | .ZZA  | labor   | 0      | I       | 1   |
#  And these "tx_requests":
#  | nvid | created | status  | amount | payer | payee | for   |*
#  |    1 | %today  | paid    |    100 | .ZZB  | .ZZA  | labor |
  
#  When member ".ZZB" visits page "history/transactions/period=15"
#  Then we show "Transaction History" with:
#  | Tx# | Date | Name    | Purpose                     | Amount | Balance |
#  |  1  | %mdy | Abe One | labor (CG inv#1 - overpaid) | 110.00 | -110.00 |

Scenario: A member tries to pay an invoice that is already paid
  Given these "tx_requests":
  | nvid | created | status | amount | payer | payee | for   |*
  |    1 | %today  | paid   |    100 | .ZZB  | .ZZA  | labor |
  When member ".ZZB" confirms form "handle-invoice/nvid=1&code=%code" with values:
  | op   | ret | nvid | payAmount | payer | payee | purpose | created | balFirst |*
  | pay  |     |    1 |       110 | .ZZB  | .ZZA  | labor   | %today  | 1    |
  Then we say "error": "inv already paid"

Scenario: A member tries to pay an invoice that is already being funded
  Given these "tx_requests":
  | nvid | created | status   | amount | payer | payee | for   | flags   |*
  |    1 | %today  | approved |    300 | .ZZB  | .ZZA  | labor | funding |
  When member ".ZZB" confirms form "handle-invoice/nvid=1&code=%code" with values:
  | op   | ret | nvid | payAmount | payer | payee | purpose | created | balFirst |*
  | pay  |     |    1 |       300 | .ZZB  | .ZZA  | labor   | %today  | 1    |
  Then we say "error": "inv already funding"
  
Scenario: A member who has a bank account approves an invoice
  When member ".ZZA" confirms form "tx/charge" with values:
  | op     | who     | amount | goods      | purpose |*
  | charge | Our Pub | 100    | %FOR_GOODS | stuff   |
  Then these "tx_requests":
  | nvid | created | status  | amount | payer | payee | for   |*
  |    1 | %today  | open    |    100 | .ZZC | .ZZA | stuff |
  And we message "invoiced you" to member ".ZZC" with subs:
  | otherName | amount | purpose |*
  | Abe One   | $100   | stuff   |

Scenario: A member confirms request to charge a not-yet member
  When member ".ZZA" confirms form "tx/charge" with values:
  | op     | who      | amount | goods          | purpose |*
  | charge | Dee Four | 100    | %FOR_GOODS     | labor   |
  Then these "tx_requests":
  | nvid | created | status  | amount | payer | payee | for   |*
  |    1 | %today  | open    |    100 | .ZZD | .ZZA | labor |
  And we message "invoiced you" to member ".ZZD" with subs:
  | otherName | amount | purpose | balance | creditLine                          |*
  | Abe One   | $100   | labor   |      $0 | $0 (based on your monthly activity) |

  When member ".ZZD" visits page "handle-invoice/nvid=1"
  Then we show "Confirm Payment" with:
  | ~question | Pay $100 to Abe One for labor |
  | ~ | Pay |
  | Reason ||
  | ~ | dispute |

  When member ".ZZD" confirms form "handle-invoice/nvid=1" with values:
  | op   | ret | nvid | payAmount | payer | payee | purpose | created |*
  | pay  |     |    1 |       100 | .ZZD  | .ZZA  | labor   | %today  |
  Then these "tx_requests":
  | nvid | created | status   | amount | payer | payee | for   |*
  |    1 | %today  | approved |    100 | .ZZD | .ZZA | labor |
  And we say "status": "finish signup|when funded"
  
Scenario: A member denies an invoice
  When member ".ZZA" confirms form "tx/charge" with values:
  | op     | who     | amount | goods | purpose |*
  | charge | Bea Two | 100    | %FOR_GOODS     | labor   |
  And member ".ZZB" confirms form "handle-invoice/nvid=1" with values:
  | op   | ret | nvid | payAmount | payer | payee | purpose | created | reason |*
  | deny |     |    1 |       100 | .ZZB  | .ZZA  | labor   | %today  | broke  |
  Then these "tx_requests":
  | nvid | created | status | amount | payer | payee | for   |*
  |    1 | %today  | denied |    100 | .ZZB  | .ZZA  | labor |
  And we message "invoice denied" to member ".ZZA" with subs:
  | payerName | created | amount | purpose | reason |*
  | Bea Two   | %mdY    |   $100 | labor   | broke  |
  And balances:
  | uid  | balance |*
  | .ZZA |       0 |
  | .ZZB |       0 |
  | .ZZC |       0 |

Scenario: A member with a bank account approves an invoice drawing automatically
  Given balances:
  | uid  | balance |*
  | .ZZB | 100     |
  When member ".ZZA" confirms form "tx/charge" with values:
  | op     | who     | amount | goods      | purpose |*
  | charge | Bea Two | 600    | %FOR_GOODS | labor   |
  And member ".ZZB" confirms form "handle-invoice/nvid=1" with values:
  | op   | ret | nvid | payAmount | payer | payee | purpose | created | reason | balFirst |*
  | pay  |     |    1 |       600 | .ZZB  | .ZZA  | labor   | %today  |        | 1    |
  Then these "tx_requests":
  | nvid | created | status   | amount | payer | payee | for   |*
  |    1 | %today  | approved |    600 | .ZZB  | .ZZA  | labor |
  And we say "error": "short to|expect a transfer" with subs:
  | short |*
  | $500  |
  # avail was $350
  And balances:
  | uid  | balance |*
  | .ZZA |       0 |
  | .ZZB |     100 |
  | .ZZC |       0 |
  And these "txs2":
  | payee | amount |*
  | .ZZB  | 500    |

Scenario: A member with a bank account approves an invoice not drawing automatically
  When member ".ZZA" confirms form "tx/charge" with values:
  | op     | who     | amount | goods      | purpose |*
  | charge | Bea Two | 600    | %FOR_GOODS | labor   |
  And member ".ZZB" confirms form "handle-invoice/nvid=1" with values:
  | op   | ret | nvid | payAmount | payer | payee | purpose | created | reason | balFirst |*
  | pay  |     |    1 |       600 | .ZZB  | .ZZA  | labor   | %today  |        | 0    |
  Then these "tx_requests":
  | nvid | created | status   | amount | payer | payee | for   |*
  |    1 | %today  | approved |    600 | .ZZB  | .ZZA  | labor |
  And we say "error": "short to|expect a transfer" with subs:
  | short |*
  | $600  |
  # avail was $250
  And balances:
  | uid  | balance |*
  | .ZZA |       0 |
  | .ZZB |       0 |
  | .ZZC |       0 |
  And these "txs2":
  | payee | amount |*
  | .ZZB  | 600    |

Scenario: A member approves an invoice with insufficient funds without a connected bank account
  Given members have:
  | uid  | risks | bankAccount |*
  | .ZZB |       |             |
  When member ".ZZA" confirms form "tx/charge" with values:
  | op     | who     | amount | goods      | purpose |*
  | charge | Bea Two | 600    | %FOR_GOODS | labor   |
  And member ".ZZB" confirms form "handle-invoice/nvid=1" with values:
  | op   | ret | nvid | payAmount | payer | payee | purpose | created | reason | balFirst |*
  | pay  |     |    1 |       600 | .ZZB  | .ZZA  | labor   | %today  |        | 0    |
  Then these "tx_requests":
  | nvid | created | status   | amount | payer | payee | for   |*
  |    1 | %today  | approved |    600 | .ZZB  | .ZZA  | labor |
  And we say "error": "short to|when funded|how to fund" with subs:
  | short |*
  | $600  |
  # avail was $0
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
  | nvid | created | status   | amount | payer | payee | for   |*
  |    1 | %today  | approved |    300 | .ZZB  | .ZZA  | labor |
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
  |   1 | %today  |    100 | .ZZB  | .ZZA  | labor     | 0      | I       | 1   |
  And these "tx_requests":
  | nvid | created | status | amount | payer | payee | for   |*
  |    1 | %today  | 1      |    100 | .ZZB  | .ZZA  | labor |
  And these "txs2":
  | txid | created | completed | amount | payee |*
  |    1 | %now    | %now      | 100    | .ZZB  |
  And balances:
  | uid  | balance |*
  | .ZZA |     100 |
  | .ZZB |       0 |
  | .ZZC |       0 |
  
Scenario: A payee marks an invoice paid manually
  Given these "tx_requests":
  | nvid | created | status  | amount | payer | payee | for   |*
  |    1 | %today  | open    |    300 | .ZZB  | .ZZA  | labor |
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
  | nvid | created | status | amount | payer | payee | for   | reason |*
  |    1 | %today  | paid   |    300 | .ZZB  | .ZZA  | labor | cuz    |
  And we message "invoice withdrawn" to member ".ZZB" with subs:
  | amount | payerName | payeeName | created | purpose | reason | done        |*
  | $300   | Bea Two   | Abe One   | %mdY    | labor   | cuz    | marked PAID |

Scenario: A payee cancels an invoice
  Given these "tx_requests":
  | nvid | created | status  | amount | payer | payee | for   |*
  |    1 | %today  | open    |    300 | .ZZB  | .ZZA  | labor |
  When member ".ZZA" confirms form "handle-invoice/nvid=1" with values:
  | op    | ret | nvid | payAmount | payer | payee | purpose | created | reason | action |*
  | close |     |    1 |       300 | .ZZB  | .ZZA  | labor   | %today  | cuz    |      1 |
  Then these "tx_requests":
  | nvid | created | status   | amount | payer | payee | for   | reason |*
  |    1 | %today  | canceled |    300 | .ZZB  | .ZZA  | labor | cuz    |
  And we message "invoice withdrawn" to member ".ZZB" with subs:
  | amount | payerName | payeeName | created | purpose | reason | done        |*
  | $300   | Bea Two   | Abe One   | %mdY    | labor   | cuz    | canceled    |

Scenario: A payee reopens an invoice
  Given these "tx_requests":
  | nvid | created | status | amount | payer | payee | for   | reason |*
  |    1 | %today  | paid   |    300 | .ZZB  | .ZZA  | labor | cuz    |
  When member ".ZZA" visits "handle-invoice/nvid=1"
  Then we show "Invoice Was Closed Manually" with:
  | Reopen invoice #1 (%mdY) charging Bea Two $300 for labor? | |
  | Closed Because | cuz    |
  |                | Reopen |
  When member ".ZZA" confirms form "handle-invoice/nvid=1" with values:
  | op     | ret | nvid | payAmount | payer | payee | purpose | created |*
  | reopen |     |    1 |       300 | .ZZB  | .ZZA  | labor   | %today  |
  Then these "tx_requests":
  | nvid | created | status   | amount | payer | payee | for   | reason |*
  |    1 | %today  | open     |    300 | .ZZB  | .ZZA  | labor | cuz    |
  And we message "invoice reopened|invoiced you" to member ".ZZB" with subs:
  | amount | otherName | otherEmail    | otherPhone      | payerName | payeeName | created | purpose |*
  | $300   | Abe One   | a@example.com | +1 413 628 0001 | Bea Two   | Abe One   |%mdY     | labor   |
