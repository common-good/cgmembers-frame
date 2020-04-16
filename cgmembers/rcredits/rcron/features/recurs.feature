Feature: Recurs
AS a member
I WANT to make recurring payments (typically gifts) to other members
SO I can save on memory and labor.

Setup:
  Given members:
  | uid  | fullName | address | city  | state | zip   | country | postalAddr | flags               | risks   | floor |*
  | .ZZA | Abe One  | 1 A St. | Atown | AK    | 01000 | US      | 1 A, A, AK | ok,confirmed,bankOk | hasBank |   -20 |
  | .ZZB | Bea Two  | 2 B St. | Btown | PA    | 01002 | US      | 2 B, B, BC | ok,confirmed,cAdmin |         |  -200 |
  | .ZZC | Cor Pub  | 3 C St. | Ctown | CT    | 03000 | US      | 3 C, C, CT | ok,co,confirmed     |         |     0 |
  And transactions:
  | xid | created   | amount | payer | payee | purpose |*
  |   1 | %today-4m |    100 | .ZZB | .ZZA | loan    |

Scenario: A brand new recurring payment can be completed
  Given these "recurs":
  | created    | payer | payee | amount | period | purpose |*
  | %yesterday | .ZZA  | .ZZB  |     10 |      W | pmt     |
  When cron runs "recurs"
  Then transactions:
  | xid | created | amount | payer | payee | purpose      | flags  |*
  |   2 | %today  |     10 | .ZZA | .ZZB | pmt (Weekly) | recurs |
  And we notice "new payment" to member ".ZZB" with subs:
  | otherName | amount | payeePurpose | aPayLink |*
  | Abe One   | $10    | pmt (Weekly) | ?        |
  And we notice "recur pay" to member ".ZZA" with subs:
  | amount | period | purpose | payee   |*
  |    $10 | Weekly | pmt     | Bea Two |
  # and many other fields
	And count "txs" is 2
	And count "usd" is 0
	And count "invoices" is 0
	When cron runs "recurs"
	Then count "txs" is 2
	And count "usd" is 0
	And count "invoices" is 0

Scenario: A second recurring payment can be completed
  Given these "recurs":
  | id | created   | payer | payee | amount | period | purpose |*
  |  8 | %today-2w | .ZZA  | .ZZB  |     10 |      W | pmt     |
  And transactions:
  | xid | created    | amount | payer | payee | purpose      | flags  | recursId |*
  |   2 | %today-32d |     10 | .ZZA | .ZZB | pmt (Weekly) | recurs |        8 |
  When cron runs "recurs"
  Then transactions:
  | xid | created | amount | payer | payee | purpose      | flags  | recursId |*
  |   3 | %today  |     10 | .ZZA | .ZZB | pmt (Weekly) | recurs |        8 |

Scenario: A recurring payment happened yesterday
  Given these "recurs":
  | id | created    | payer | payee | amount | period | purpose |*
  |  8 | %yesterday | .ZZA  | .ZZC  |     10 |      M | pmt     |
  And transactions:
  | xid | created    | amount | payer | payee | purpose       | flags  | recursId |*
  |   2 | %yesterday |     10 | .ZZA | .ZZC | pmt (Monthly) | recurs |        8 |
  When cron runs "recurs"
  Then count "txs" is 2
  
Scenario: A recurring payment happened long enough ago to repeat
  Given these "recurs":
  | id | created        | payer | payee | amount | period | purpose |*
  |  8 | %yesterday-35d | .ZZA  | .ZZC  |     10 |      M | pmt     |
  And transactions:
  | xid | created    | amount | payer | payee | purpose       | flags  | recursId |*
  |   2 | %today-35d |     10 | .ZZA | .ZZC | pmt (Monthly) | recurs |        8 |
  When cron runs "recurs"
  Then transactions:
  | xid | created | amount | payer | payee | purpose       | flags  | recursId |*
  |   3 | %today  |     10 | .ZZA | .ZZC | pmt (Monthly) | recurs |        8 |
  And count "txs" is 3
  And count "invoices" is 0
  
Scenario: A recurring payment cannot be completed
  Given these "recurs":
  | id | created    | payer | payee | amount | period | purpose |*
  |  8 | %yesterday | .ZZA  | .ZZB  |    200 |      W | pmt     |
  When cron runs "recurs"
	Then invoices:
  | nvid | created   | status       | amount | payer | payee | for          | flags  | recursId |*
  |    1 | %today    | %TX_APPROVED |    200 | .ZZA | .ZZB | pmt (Weekly) | recurs |	      8 |
	And count "txs" is 1
	And count "usd" is 0
	And count "invoices" is 1

  When cron runs "invoices"
  Then these "usd":
  | txid | amount | payee | completed | deposit |*
  |    1 |    200 | .ZZA  |         0 |       0 |
	Then count "txs" is 2
  And count "usd" is 1
  And count "invoices" is 1
  And	invoices:
  | nvid | created   | status       | amount | payer | payee | for          | flags          | recursId |*
  |    1 | %today    | %TX_APPROVED |    200 | .ZZA | .ZZB | pmt (Weekly) | recurs,funding |	       8 |

	When cron runs "recurs"
	Then count "txs" is 2
  And count "usd" is 1
  And count "invoices" is 1

Skip because member should be allowed to be invoiced?
Scenario: A recurring payment invoice cannot be completed because member is uncarded
  Given invoices:
  | nvid | created   | status       | amount | payer | payee | for | flags  |*
  |    1 | %today    | %TX_APPROVED |     50 | .ZZA | .ZZB | pmt | recurs |
  And member ".ZZA" has no photo ID recorded
  When cron runs "invoices"
	Then count "txs" is 1
	And count "invoices" is 1
Resume
