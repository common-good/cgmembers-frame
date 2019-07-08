Feature: Transact
AS a member
I WANT to join the Investment Club and invest
SO I can support local initiatives and get a return on my savings.
 
Setup:
  Given members:
  | uid  | fullName | floor | flags                     | state |*
  | .ZZA | Abe One  |  -250 | ok,confirmed,debt         |    MA |
  | .ZZB | Bea Two  |  -250 | ok,confirmed,debt,icadmin |    MA |
  | .ZZC | Our Pub  |  -250 | ok,confirmed,debt,co     	|    MA |
  | .ZZD | Dee Four |  -250 | ok,confirmed,debt         |    MA |
  | .ZZE | Eve Five |  -250 | ok,confirmed,debt         |    MA |
  | .ZZF | Fred Six |  -250 | ok,confirmed,debt         |    MA |
	| .ZZI | In Club  |     0 | ok,confirmed,co,icadmin   |    MA |
	# for now, all MA accounts have the same iclub
  
  And relations:
  | main | other | permission |*
  | .ZZC | .ZZA  | manage     |

Scenario: A member joins the investment club
  When member ".ZZA" visits page "invest"
	Then we show "Join Your"

  When member ".ZZA" completes form "invest" with values:
  | signedBy |*
  | Abe One  |
  And member ".ZZA" visits page "invest"
  Then we show "Investment Club" with:
  | List Investments ||
	| Club Value: | $0 ($0 liquid) |
	| Your Share: | $0 (0.00%) |
	| Invest: |  |
  | Invest MORE ||
  And we say "status": "now a member"
  And these "stakes":
  | stakeid | uid  | clubid | stake | joined |*
  |       1 | .ZZA | .ZZI   |     0 | %now   |

Scenario: A member company joins the investment club
  When member "C:A" visits page "invest"
	Then we show "Join Your"

  When member "C:A" completes form "invest" with values:
  | signedBy |*
  | Abe One  |
  And member "C:A" visits page "invest"
  Then we show "Investment Club" with:
	| Club Value | $0 ($0 liquid) |
	| Your Share | $0 (0.00%) |
	| Invest |  |
  And we say "status": "now a member"
  
Scenario: A member buys a stake in the club
  Given transactions:
  | amount | from | to   |*
  |    100 | .ZZE | .ZZA |
  And these "stakes":
  | stakeid | uid  | clubid | stake | joined  |*
  |       1 | .ZZA | .ZZI   |     0 | %now-2d |
  Then balances:
  | uid  | balance |*
  | .ZZA |     100 |
  When member ".ZZA" completes form "invest" with values:
  | op  | amount |*
  | buy |     10 |
  Then we say "status": "report tx|investment increase" with subs:
  | did  | otherName | amount |*
  | paid | In Club   | $10    |
  And these "stakes":
  | stakeid | uid  | clubid | stake | joined  |*
  |       1 | .ZZA | .ZZI   |    10 | %now-2d |
  
  When member ".ZZA" completes form "invest" with values:
  | op  | amount |*
  | buy |     23 |
  Then these "stakes":
  | stakeid | uid  | clubid | stake | joined  |*
  |       1 | .ZZA | .ZZI   |    33 | %now-2d |

Scenario: The club adds a proposed investment

Scenario: Memmbers rate a proposed investment

Scenario: The club buys shares

Scenario: The club sells shares

Scenario: Members increase and decrease their stakes
