Feature: Gifts
AS a member
I WANT my recent requested donation to CGF to go through
SO I can enjoy the rCredit system's rapid growth and be a part of that.

Setup:
  Given members:
  | uid  | fullName | address | city  | state | zip   | country | postalAddr | flags               | risks   | floor |*
  | .ZZA | Abe One  | 1 A St. | Atown | AK    | 01001 | US      | 1 A, A, AK | ok,confirmed,bankOk | hasBank |   -20 |
  | .ZZB | Bea Two  | 2 B St. | Btown | PA    | 01002 | US      | 2 B, B, BC | ok,confirmed,cAdmin |         |  -200 |
  And transactions:
  | xid | created   | amount | payer | payee | purpose |*
  |   1 | %today-4m |    100 | .ZZB | .ZZA | loan    |

Scenario: A donation to CG is visible to admin
  Given these "recurs":
  | created | payer | payee | amount | period | purpose |*
  | %today  | .ZZA  | cgf   |     10 |      W | gift!   |
  When member "A:B" visits page ""
  Then we show "Summary" with:
  | Donations: | $10 Weekly |

Scenario: A brand new recurring donation to CG can be completed
  Given these "recurs":
  | id | created    | payer | payee | amount | period | purpose |*
  |  7 | %yesterday | .ZZA  | cgf   |     10 |      M | gift!   |
  When cron runs "recurs"
  Then transactions:
  | xid | created | amount | payer | payee | purpose         | flags       | recursId |*
  |   2 | %today  |     10 | .ZZA | cgf | gift! (Monthly) | gift,recurs |        7 |
  And we notice "new payment linked" to member "cgf" with subs:
  | otherName | amount | payeePurpose    | aPayLink |*
  | Abe One   | $10    | gift! (Monthly) | ?        |
  And that "notice" has link results:
  | ~name | Abe One |
  | ~postalAddr | 1 A, A, AK |
  | Physical address: | 1 A St., Atown, AK 01001 |
  | ~footer | %PROJECT |
  And we notice "recur pay" to member ".ZZA" with subs:
  | amount | period  | purpose | payee    |*
  |    $10 | Monthly | gift!   | %PROJECT |
  # and many other fields
	And count "txs" is 2
	And count "usd" is 0
	And count "invoices" is 0
	When cron runs "recurs"
	Then count "txs" is 2
	And count "usd" is 0
	And count "invoices" is 0

Scenario: A second recurring donation to CG can be completed
  Given these "recurs":
  | created   | payer | payee | amount | period | purpose |*
  | %today-3m | .ZZA  | cgf   |     10 |      M | gift!   |
  And transactions:
  | xid | created    | amount | payer | payee | purpose         | flags       |*
  |   1 | %today-32d |     10 | .ZZA | cgf | gift! (Monthly) | gift,recurs |
  When cron runs "recurs"
  Then transactions:
  | xid | created | amount | payer | payee | purpose         | flags          |*
  |   2 | %today  |     10 | .ZZA | cgf | gift! (monthly) | gift,recurs |

Scenario: A donation invoice (to CG) can be completed
# even if the member has never yet made a cgCard purchase
  Given these "recurs":
  | id | created    | payer | payee | amount | period | purpose |*
  |  8 | %yesterday | .ZZA  | cgf   |     10 |      M | gift!   |
  And invoices:
  | nvid | created   | status       | amount | payer | payee | for      | flags       | recursId |*
  |    2 | %today    | %TX_APPROVED |     50 | .ZZA | cgf | donation | gift,recurs |        8 |
  And member ".ZZA" has no photo ID recorded
  When cron runs "invoices"
  Then transactions: 
  | xid | created | amount | payer | payee | purpose                      | flags       | recursId |*
  |   2 | %today  |     50 | .ZZA | cgf | donation (Common Good inv#2) | gift,recurs |        8 |
	And invoices:
  | nvid | created   | status | purpose  |*
  |    2 | %today    | 2      | donation |

Scenario: A recurring donation to CG cannot be completed
  Given these "recurs":
  | created   | payer | payee | amount | period | purpose |*
  | %today-3m | .ZZA  | cgf   |    200 |      M | gift!   |
  When cron runs "recurs"
	Then invoices:
  | nvid | created   | status       | amount | payer | payee | for             | flags          |*
  |    1 | %today    | %TX_APPROVED |    200 | .ZZA | cgf | gift! (Monthly) | gift,recurs |	
	And count "txs" is 1
	And count "usd" is 0
	And count "invoices" is 1

  When cron runs "invoices"
	Then count "txs" is 2
  And count "usd" is 1
  And count "invoices" is 1
  And	invoices:
  | nvid | created   | status       | amount | payer | payee | for             | flags               |*
  |    1 | %today    | %TX_APPROVED |    200 | .ZZA | cgf | gift! (Monthly) | gift,recurs,funding |	

	When cron runs "recurs"
	Then count "txs" is 2
  And count "usd" is 1
  And count "invoices" is 1

Scenario: A non-member chooses a donation to CG
  Given members:
  | uid  | fullName | flags  | risks   | activated | balance |*
  | .ZZD | Dee Four |        | hasBank |         0 |       0 |
  | .ZZE | Eve Five | refill | hasBank | %today-9m |     200 |
  And these "recurs":
  | created   | payer | payee | amount | period |*
  | %today-3y | .ZZD  | cgf   |      1 |      Y |
  | %today-3m | .ZZE  | cgf   |    200 |      M |
  When cron runs "recurs"
	Then count "txs" is 1
	And count "usd" is 0
	And count "invoices" is 0

Scenario: It's time to warn about an upcoming annual donation to CG
  Given members:
  | uid  | fullName | flags  | risks   | activated               |*
  | .ZZD | Dee Four | ok     | hasBank | %now-1y                 |
  | .ZZE | Eve Five | ok     | hasBank | %(strtotime('+7 days', strtotime('-1 year', %daystart))) |
  And these "recurs":
  | id | created               | payer | payee | amount | period | purpose |*
  |  1 | %(strtotime('+7 days', strtotime('-1 year', %daystart))) | .ZZD  | cgf   |      1 |      Y | gift!   |
	And transactions:
  | xid | created               | amount | payer | payee | purpose                    | flags       | recursId |*
  |   1 | %(strtotime('+7 days', strtotime('-1 year', %daystart))) | 10 | .ZZD | cgf | gift! (Yearly) | gift,recurs | 1 |
  When cron runs "tickle"
	Then we email "annual-gift" to member "d@example.com" with subs:
	| amount | when    | aDonate |*
	|     $1 | %mdY+7d |       ? |
	And we email "annual-gift" to member "e@example.com" with subs:
	| amount | when    | aDonate |*
	|     $0 | %mdY+7d |       ? |	
