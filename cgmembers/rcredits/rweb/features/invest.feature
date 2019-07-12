Feature: Transact
AS a member
I WANT to join the Investment Club and invest
SO I can support local initiatives and get a return on my savings.
 
Setup:
  Given members:
  | uid  | fullName        | floor | flags                | city | zip   |*
  | .ZZA | Abe One         |  -250 | ok,confirmed,debt    | Aton | 01000 |
  | .ZZB | Bea Two         |  -250 | ok,confirmed,debt    | Bton | 01000 |
  | .ZZC | Our Pub         |  -250 | ok,confirmed,debt,co | Cton | 01000 |
  | .ZZD | Dee Four        |  -250 | ok,confirmed,debt    | Dton | 01000 |
  | .ZZE | Eve Five        |  -250 | ok,confirmed,debt    | Eton | 01000 |
  | .ZZF | Fred Six        |  -250 | ok,confirmed,debt    | Fton | 01000 |
	| .ZZI | Investment Club |     0 | ok,confirmed,co      | Iton | 01*   |
  
  And relations:
  | main | other | permission |*
  | .ZZC | .ZZA  | manage     |
  | .ZZI | .ZZB  | manage     |
Skip
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
  | paid | Investment Club   | $10    |
  And these "stakes":
  | stakeid | uid  | clubid | stake | joined  |*
  |       1 | .ZZA | .ZZI   |    10 | %now-2d |
  
  When member ".ZZA" completes form "invest" with values:
  | op  | amount |*
  | buy |     23 |
  Then these "stakes":
  | stakeid | uid  | clubid | stake | joined  |*
  |       1 | .ZZA | .ZZI   |    33 | %now-2d |

Scenario: An administrator views the investment page
  When member "I:B" visits page "invest"
  Then we show "Investment Club" with:
  | List Investments | Propose | Requests to Cash Out |
  | Member Count:         | 0  | |
	| Investments:          | $0 | |
  | Liquid:               | $0 | |
	| Loss/Expense Reserve: | $0 | |
	| Club Net Value:       | $0 | |
  | Save                  |    | |

Scenario: The club adds a proposed investment
  When member "I:B" visits page "invest/propose"
  Then we show "Propose a New Investment" with:
  | Company:    |
  | Investment: |
  | Target:     |
  # and a lot more fields
  
  When member "I:B" completes form "invest/propose" with values:
  | company | investment |equity |offering |price |return | terms | assets   | character |strength |web |history |soundness |*
  | .ZZC    | improve it |     1 | $10,000 |   10 |   4.6 | Terms | $951,000 | trusty    |      75 | 60 |     80 |       90 |
  Then we say "status": "investment proposal saved"
  And these "investments":
  | vestid | coid | clubid | proposedBy | investment | return | types | terms | assets | offering | price | character | strength | web | history | soundness | reserve |*

  |      1 | .ZZC | .ZZI   | .ZZB       | improve it | 0.046  | D     | Terms | 951000 |    10000 |    10 | trusty    |       75 |  60 |      80 |        90 |       0 |

  And we show "Proposed Investments" with:
  | Investment          | Type   | Return | Sound | Good |
  | Our Pub: improve it | equity |   4.6% |    90 | ?    |
Resume
Scenario: Members rate a proposed investment
  Given these "stakes":
  | stakeid | uid  | clubid | stake | joined  |*
  |       1 | .ZZA | .ZZI   |    33 | %now-2d |
  And these "investments":
  | vestid | coid | clubid | proposedBy | investment | return | types | terms | assets | offering | price | character | strength | web | history | soundness | reserve |*
  |      1 | .ZZC | .ZZI   | .ZZB       | improve it | 0.046  | D     | Terms | 951000 |    10000 |    10 | trusty    |       75 |  60 |      80 |        90 |       0 |
  When member ".ZZA" visits page "invest/rate/vestid=1&clubqid=NEWZZI"
  Then we show "View or Rate Investment #1" with:
  | Numeric ratings are on a scale of | 0-100 |
  | Company | Our Pub |
  | City | Cton |
  | Service area | |
  | Founded | 07/10/2019 |
  | Annual gross | $0 |
  | Co description | |
  | Project/Purpose | improve it |
  | Offering size | $10,000 |
  | Share price | 10.00 |
  | Predicted return | 0.05% |
  | Terms | Terms |
  | Company assets | $951,000 |
  | Owner character | trusty |
  | Financial strength | 75 |
  | Web presence | 60 |
  | Repayment history | 80 |
  | Overall soundness | 90 |
  | Common Goodness | |
  | Rate this investment's benefit to the community and the common good | (0-100) |
  | Your patronage | |
  | Comments | |
  | Rate | |

  Given these "ratings":
  | ratingid | vestid | uid  | good | comment | patronage |*
  |        1 |      1 | .ZZD |   40 | yay!    |        80 |
  When member ".ZZA" completes form "invest/rate/vestid=1&clubqid=NEWZZI" with values:
  | good | patronage | comment |*
  |   20 |        50 | do it!  |
  Then we say "status": "rating successful" with subs:
  | num |*
  |   1 |
  And these "ratings":
  | ratingid | vestid | uid  | good | comment | patronage |*
  |        2 |      1 | .ZZA |   20 | do it!  |        50 |
  And we show "Proposed Investments" with:
  | Investment          | Type   | Return | Sound | Good |
  | Our Pub: improve it | equity |   4.6% |    90 |   30 |
  
Skip  
Scenario: The club buys shares

Scenario: The club sells shares

Scenario: Members increase and decrease their stakes
