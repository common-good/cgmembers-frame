Feature: Invest
AS a member
I WANT to join the Investment Club and invest
SO I can support local initiatives and get a return on my savings.
 
Setup:
  Given members:
  | uid  | fullName        | floor | flags                      | city | zip   | dob     | federalId |*
  | .ZZA | Abe One         |  -250 | ok,confirmed,debt          | Aton | 01000 | %now    | 123456789 |
  | .ZZB | Bea Two         |  -250 | ok,confirmed,debt,reinvest | Bton | 01000 | %now    | 123456789 |
  | .ZZC | Our Pub         |  -250 | ok,confirmed,debt,co       | Cton | 01000 | 0       | 123456789 |
  | .ZZD | Dee Four        |  -250 | ok,confirmed,debt          | Dton | 01000 | %now    | 123456789 |
  | .ZZE | Eve Five        |  -250 | ok,confirmed,debt          | Eton | 01000 | %now    | 123456789 |
  | .ZZF | Fred Six        |  -250 | ok,confirmed,debt          | Fton | 01000 | %now    | 123456789 |
  | .ZZI | Investment Club |     0 | ok,confirmed,co            | Iton | 01*   | %now    | 123456789 |
  And members have:
  | uid  | founded |*
  | .ZZC | %now-1d |

  And these "u_relations":
  | main | other | permission |*
  | .ZZC | .ZZA  | manage     |
  | .ZZI | .ZZB  | manage     |
  
  And balances:
  | uid  | balance |*
  | .ZZA |     200 |
  | .ZZB |     200 |
  |  cgf |    -400 |
  
  And member ".ZZA" has "person" steps done: "ssn contact crumbs"
  And member ".ZZB" has "person" steps done: "ssn contact crumbs"

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
  And these "r_stakes":
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
  Given these "r_stakes":
  | stakeid | uid  | clubid | stake | joined  |*
  |       1 | .ZZA | .ZZI   |     0 | %now-2d |
  When member ".ZZA" completes form "invest" with values:
  | op  | amount |*
  | buy |     10 |
  Then these "txs":
  | xid | created | amount | payer | payee | purpose    | flags |*
  |   1 | %now    |     10 | .ZZA  | .ZZI | investment | stake |
  And we say "status": "report tx|investment increase" with subs:
  | did  | otherName       | amount |*
  | paid | Investment Club | $10    |
  And these "r_stakes":
  | stakeid | uid  | clubid | stake | joined  |*
  |       1 | .ZZA | .ZZI   |    10 | %now-2d |
  
  When member ".ZZA" completes form "invest" with values:
  | op  | amount |*
  | buy |     23 |
  Then these "r_stakes":
  | stakeid | uid  | clubid | stake | joined  |*
  |       1 | .ZZA | .ZZI   |    33 | %now-2d |

Scenario: An administrator views the investment page
  When member "I:B" visits page "invest"
  Then we show "Investment Club" with:
  | Liquid:                   |  $0 | ||
  | Requests to Cash Out (0): |  $0 | Cash Out ||
   | Investments (0):          |  $0 | List | Propose |
  | Loss Reserve:             |  $0 | ||
  | Expense Reserve:          |  $0 | Save ||
  | Club Net Value:           |  $0 | ||
  | Total Member Stakes (0):  |  $0 | ||
  | Available for Dividends:  |  $0 | Issue Dividends ||

Scenario: The club adds a proposed investment
  When member "I:B" visits page "invest/propose"
  Then we show "Propose a New Investment" with:
  | Company:    |
  | Investment: |
  | Target:     |
  # and a lot more fields
  
  When member "I:B" completes form "invest/propose" with values:
  | company       | investment |equity |offering |price |return | terms | assets   | character |strength |web |history |soundness |*
  | c@example.com | improve it |     1 | $10,000 |   10 |   4.6 | Terms | $951,000 | trusty    |      75 | 60 |     80 |       90 |
  Then we say "status": "investment proposal saved"
  And these "r_investments":
  | vestid | coid | clubid | proposedBy | investment | return | types | terms | assets | offering | price | character | strength | web | history | soundness | reserve |*

  |      1 | .ZZC | .ZZI   | .ZZB       | improve it | 0.046  | D     | Terms | 951000 |    10000 |    10 | trusty    |       75 |  60 |      80 |        90 |       0 |

  And we show "Club Investments" with:
  | Status   | Investment          | Type   | Return | Value | Good |
  | proposed | Our Pub: improve it | Equity |  4.6%  |       | ?    |
  
  When member "I:B" visits page "invest/rate/vestid=1&clubqid=NEWZZI"
  Then we show "Investment #1" with:
  | Goodness  | 0 (med 0) |
  | Company   | Our Pub |
  | Project   | improve it |
  | Type      | Equity |
  | return    | 4.6% |
  | soundness | 90 |

Scenario: Members rate a proposed investment
  Given these "r_stakes":
  | stakeid | uid  | clubid | stake | joined  |*
  |       1 | .ZZA | .ZZI   |    33 | %now-2d |
  And these "r_investments":
  | vestid | coid | clubid | proposedBy | investment | return | types | terms | assets | offering | price | character | strength | web | history | soundness | reserve |*
  |      1 | .ZZC | .ZZI   | .ZZB       | improve it | 0.046  | D     | Terms | 951000 |    10000 |    10 | trusty    |       75 |  60 |      80 |        90 |       0 |
  When member ".ZZA" visits page "invest/rate/vestid=1&clubqid=NEWZZI"
  Then we show "View or Rate Investment #1" with:
  | Numeric ratings are on a scale of | 0-100 |
  | Company | Our Pub |
  | City | Cton |
  | Service area | |
  | Founded | %mdY-1d |
  | Annual gross | $0 |
  | Co description | |
  | Project/Purpose | improve it |
  | Offering size | $10,000 |
  | Share price | 10.00 |
  | Predicted return | 4.6% |
  | Terms | Terms |
  | Company assets | $951,000 |
  | Owner character | trusty |
  | Financial strength | 75 |
  | Web presence | 60 |
  | Repayment history | 80 |
  | Overall soundness | 90 |
  | Common Goodness | Rate this investment's benefit |
  | Your patronage | |
  | Comments | |
  | | Rate it |

  Given these "r_ratings":
  | ratingid | vestid | uid  | good | comment | patronage |*
  |        1 |      1 | .ZZD |   40 | yay!    |        80 |
  When member ".ZZA" completes form "invest/rate/vestid=1&clubqid=NEWZZI" with values:
  | good | patronage | comment |*
  |   20 |        50 | do it!  |
  Then we say "status": "rating successful" with subs:
  | num |*
  |   1 |
  And these "r_ratings":
  | ratingid | vestid | uid  | good | comment | patronage |*
  |        2 |      1 | .ZZA |   20 | do it!  |        50 |
  And we show "Club Investments" with:
  | Investment          | Type   | Return | Value | Good |
  | Our Pub: improve it | Equity |  4.6% |       |   30 |
  
  When member ".ZZA" visits page "invest/rate/vestid=1&clubqid=NEWZZI"
  Then we show "View or Rate Investment #1" with:
  | Company | Our Pub |
  | Your rating | 20 |
  | Your patronage | 50 |
  | Your comment | do it! |
  And without:
  | | Rate it |
  
Scenario: The club buys shares
  Given these "r_stakes":
  | stakeid | uid  | clubid | stake | joined  |*
  |       1 | .ZZA | .ZZI   |   200 | %now-2d |
  And member ".ZZA" completes form "invest" with values:
  | op  | amount |*
  | buy |    200 |
  And these "r_investments":
  | vestid | coid | clubid | proposedBy | investment | return | types | terms | assets | offering | price | character | strength | web | history | soundness | reserve |*
  |      1 | .ZZC | .ZZI   | .ZZB       | improve it | 0.046  | D     | Terms | 951000 |    10000 |    10 | trusty    |       75 |  60 |      80 |        90 |       0 |

  When member "I:B" visits page "invest/rate/vestid=1&clubqid=NEWZZI"
  Then we show "Investment #1" with:
  | Numeric ratings are on a scale of | 0-100 |
  | Common Goodness | |
  | Patronage | |
  | Company | Our Pub |
  | City | Cton |
  | Service area | |
  | Founded | %mdY-1d |
  | Annual gross | $0 |
  | Co description | |
  | Project/Purpose | improve it |
  | Offering size | $10,000 |
  | Share price | 10.00 |
  | Predicted return | 4.6% |
  | Terms | Terms |
  | Company assets | $951,000 |
  | Owner character | trusty |
  | Financial strength | 75 |
  | Web presence | 60 |
  | Repayment history | 80 |
  | Overall soundness | 90 |
  | Comments | |
  And without:
  | | Rate this investment's benefit |
  | Your patronage | |
  | | Rate it |
  
  When member "I:B" visits page "invest/buy-or-sell/vestid=1"
  Then we show "Buy or Sell Investment #1" with:
  | increase or decrease the club's shares in Our Pub | |
  | Current Shares | 0 |
  | Shares to Buy | |
  | | Buy Shares |
  And without:
  | | Sell Shares |
  
  When member "I:B" completes form "invest/buy-or-sell/vestid=1" with values:
  | op  | shares |*
  | buy |     10 |
  Then we say "status": "report tx|purchased shares" with subs:
  | did  | otherName | amount | amt | co      | price |*
  | paid | Our Pub   |   $100 |  10 | Our Pub | $10   |
  And these "r_shares":
  | shid | vestid | shares | pending | when | sold |*
  |    1 |      1 |     10 |       0 | %now |      |
  And we show "Club Investments" with:
  | Investment          | Type   | Return | Value |
  | Our Pub: improve it | Equity |  4.6% | 100   |
  
  When member "I:B" visits page "invest/buy-or-sell/vestid=1"
  Then we show "Buy or Sell Investment #1" with:
  | increase or decrease the club's shares in Our Pub | |
  | Current Shares | 10 (at $10) |
  | Pending Sales | 0 |
  | Buy or Sell | |
  | | Buy Shares |
  | | Sell Shares |

Scenario: The club sells shares
  Given these "r_stakes":
  | stakeid | uid  | clubid | stake | joined  |*
  |       1 | .ZZA | .ZZI   |   200 | %now-2d |
  And member ".ZZA" completes form "invest" with values:
  | op  | amount |*
  | buy |    200 |
  And these "r_investments":
  | vestid | coid | clubid | proposedBy | investment | return | types | terms | assets | offering | price | character | strength | web | history | soundness | reserve |*
  |      1 | .ZZC | .ZZI   | .ZZB       | improve it | 0.046  | D     | Terms | 951000 |    10000 |    10 | trusty    |       75 |  60 |      80 |        90 |       0 |
  And these "r_shares":
  | shid | vestid | shares | pending | when |*
  |    1 |      1 |     10 |       0 | %now |
  
  # now request sale of 4 of the 10 shares
  When member "I:B" completes form "invest/buy-or-sell/vestid=1" with values:
  | op   | shares |*
  | sell |      4 |
  Then we say "status": "report tx|balance unchanged|investment sale pending" with subs:
  | did     | otherName | amount | co      |*
  | charged | Our Pub   |    $40 | Our Pub |
  And these "tx_requests":
  | nvid | payer | payee | amount | purpose                               | flags      |*
  |    1 | .ZZC  | .ZZI  |     40 | redeeming investment: 4 shares at $10 | investment |
  And these "r_shares":
  | shid | vestid | shares | pending | when | sold |*
  |    1 |      1 |     10 |       0 | %now |      |
  |    2 |      1 |      0 |      -4 | %now |      |
  When member "I:B" visits page "invest/buy-or-sell/vestid=1"
  Then we show "Buy or Sell Investment #1" with:
  | increase or decrease the club's shares in Our Pub | |
  | Current Shares | 10 (at $10) |
  | Pending Sales | 4 |
  | Buy or Sell | |
  | | Buy Shares |
  | | Sell Shares |  

  # finish sale
  When member "C:A" completes form "handle-invoice/nvid=1&code=TESTDOCODE" with values:
  | op  |*
  | pay |
  Then these "r_shares":
  | shid | vestid | shares | pending | when | sold |*
  |    1 |      1 |     10 |       0 | %now |      |
  |    2 |      1 |     -4 |       0 | %now |      |

  When member ".ZZA" visits page "invest/list/clubqid=NEWZZI"
  Then we show "Club Investments" with:
  | Investment          | Type   | Return | Value |
  | Our Pub: improve it | Equity |  4.6% | 60    |
  When member "I:B" visits page "invest/buy-or-sell/vestid=1"
  Then we show "Buy or Sell Investment #1" with:
  | increase or decrease the club's shares in Our Pub | |
  | Current Shares | 6 (at $10) |
  | Pending Sales | 0 |
  | Buy or Sell | |
  | | Buy Shares |
  | | Sell Shares |  
  
Scenario: The club sells its remaining shares in an investment
  Given these "r_stakes":
  | stakeid | uid  | clubid | stake | joined  |*
  |       1 | .ZZA | .ZZI   |   200 | %now-2d |
  And member ".ZZA" completes form "invest" with values:
  | op  | amount |*
  | buy |    200 |
  And these "r_investments":
  | vestid | coid | clubid | proposedBy | investment | return | types | terms | assets | offering | price | character | strength | web | history | soundness | reserve |*
  |      1 | .ZZC | .ZZI   | .ZZB       | improve it | 0.046  | D     | Terms | 951000 |    10000 |    10 | trusty    |       75 |  60 |      80 |        90 |       0 |
  And these "r_shares":
  | shid | vestid | shares | pending | when |*
  |    1 |      1 |     10 |       0 | %now |
  |    2 |      1 |     -4 |       0 | %now |

  # sell the last 6 shares
  When member "I:B" completes form "invest/buy-or-sell/vestid=1" with values:
  | op   | shares |*
  | sell |      6 |
  Then we say "status": "report tx|balance unchanged|investment sale pending" with subs:
  | did     | otherName | amount | co      |*
  | charged | Our Pub   |    $60 | Our Pub |
  And these "tx_requests":
  | nvid | payer | payee | amount | purpose                               | flags      |*
  |    1 | .ZZC  | .ZZI  |     60 | redeeming investment: 6 shares at $10 | investment |  
  When member "C:A" completes form "handle-invoice/nvid=1&code=TESTDOCODE" with values:
  | op  |*
  | pay |
  Then these "r_shares":
  | shid | vestid | shares | pending | when | sold |*
  |    1 |      1 |     10 |       0 | %now | %now |
  |    2 |      1 |     -4 |       0 | %now | %now |
  |    3 |      1 |     -6 |       0 | %now | %now | 
  
Scenario: Members increase and decrease their stakes
  Given these "txs":
  | xid | created | amount | payer | payee | purpose    | flags |*
  |   1 | %now-3d |     10 | .ZZA  | .ZZI | investment | stake |
  |   2 | %now-1d |     70 | .ZZB  | .ZZI | investment | stake |
  And these "r_stakes":
  | stakeid | uid  | clubid | stake | joined  |*
  |       1 | .ZZA | .ZZI   |    10 | %now-2d |
  |       2 | .ZZB | .ZZI   |    70 | %now-2d |
  And these "r_investments":
  | vestid | coid | clubid | proposedBy | investment | return | types | terms | assets | offering | price | character | strength | web | history | soundness | reserve |*
  |      1 | .ZZC | .ZZI   | .ZZB       | improve it | 0.046  | D     | Terms | 951000 |    10000 |    10 | trusty    |       75 |  60 |      80 |        90 |      10 |
  And these "r_shares":
  | shid | vestid | shares | pending | when    |*
  |    1 |      1 |    100 |       0 | %now-2m |
  |    2 |      1 |    -40 |       0 | %now-1m |
  And balances:
  | uid  | balance |*
  | .ZZI |    5000 |

  When member ".ZZA" visits page "invest"
  Then we show "Investment Club" with:
  | Club Value | $5,590 ($5,000 liquid) |
  | Your Share | $1,677 (30.0%) |
  | Buy or sell: | in addition to your current request |
  | Invest MORE | Invest LESS |

  When member ".ZZA" completes form "invest" with values:
  | op  | amount |*
  | buy |     20 |
  Then we say "status": "report tx|investment increase" with subs:
  | did  | otherName       | amount |*
  | paid | Investment Club | $20    |
  And these "r_stakes":
  | stakeid | uid  | clubid | stake | joined  |*
  |       1 | .ZZA | .ZZI   |    30 | %now-2d |

  When member ".ZZA" completes form "invest" with values:
  | op   | amount |*
  | sell |     20 |
  Then we say "status": "redemption request" with subs:
  | request |*
  | $20     |
  And these "r_stakes":
  | stakeid | uid  | clubid | stake | request | joined  |*
  |       1 | .ZZA | .ZZI   |    30 |     -20 | %now-2d |
  
  When member ".ZZA" visits page "invest"
  Then we show "Investment Club" with:
  | Club Value | $5,610 ($5,020 liquid) |
  | Your Share | $1,683 (30.0%) |
  | Change request | $-20 |
  | Buy or sell: | in addition to your current request |
  | Invest MORE | Invest LESS |
  
  When member "I:B" visits page "invest"
  Then we show "Investment Club" with:
  | Liquid:                   | $5,020 | ||
  | Requests to Cash Out (1): |    $20 | Cash Out ||
   | Investments (1):          |   $600 | List | Propose |
  | Loss Reserve:             |    $10 | ||
  | Expense Reserve:          |     $0 | Save ||
  | Club Net Value:           | $5,610 | ||
  | Total Member Stakes (2):  |   $100 | ||
  | Available for Dividends:  | $5,510 | Issue Dividends ||

Scenario: A member tries to decrease stake below zero
  Given these "r_stakes":
  | stakeid | uid  | clubid | stake | request | joined  |*
  |       1 | .ZZA | .ZZI   |    10 |       0 | %now-2d |
  When member ".ZZA" completes form "invest" with values:
  | op   | amount |*
  | sell |     20 |
  Then we say "error": "investment oversell"
  And these "r_stakes":
  | stakeid | uid  | clubid | stake | request | joined  |*
  |       1 | .ZZA | .ZZI   |    10 |       0 | %now-2d |

Scenario: A club administrator handles requests to cash out
  Given these "txs":
  | xid | created | amount | payer | payee | purpose    | flags |*
  |   1 | %now-3d |     10 | .ZZA  | .ZZI | investment | stake |
  |   2 | %now-1d |     70 | .ZZB  | .ZZI | investment | stake |
  And these "r_stakes":
  | stakeid | uid  | clubid | stake | request | joined  |*
  |       1 | .ZZA | .ZZI   |    30 |     -20 | %now-2d |
  |       2 | .ZZB | .ZZI   |    70 |     -40 | %now-2d |
  And these "r_investments":
  | vestid | coid | clubid | proposedBy | investment | return | types | terms | assets | offering | price | character | strength | web | history | soundness | reserve |*
  |      1 | .ZZC | .ZZI   | .ZZB       | improve it | 0.046  | D     | Terms | 951000 |    10000 |    10 | trusty    |       75 |  60 |      80 |        90 |       0 |
  And these "r_shares":
  | shid | vestid | shares | pending | when    |*
  |    1 |      1 |    100 |       0 | %now-2m |
  |    2 |      1 |    -40 |       0 | %now-1m |
  And balances:
  | uid  | balance |*
  | .ZZI |    5000 |
  When member "I:B" visits page "invest/cashout"
  Then we show "Handle Requests to Cash Out Investments" with:
  | Available Funds | $5,000 |
  | Total Requests | $60 (2) |
  | | Cash Them Out |
  And without:
  | Method |
  
  When member "I:B" completes form "invest/cashout" with values:
  | op     |*
  | submit |
  Then we say "status": "The club paid 2 members a total of $60."
  And these "txs":
  | xid | created | amount | payer | payee | purpose           | flags |*
  |   3 | %now    |     20 | .ZZI  | .ZZA | redeem investment | stake |
  |   4 | %now    |     40 | .ZZI  | .ZZB | redeem investment | stake |
  And these "r_stakes":
  | stakeid | uid  | clubid | stake | request | joined  |*
  |       1 | .ZZA | .ZZI   |    10 |       0 | %now-2d |
  |       2 | .ZZB | .ZZI   |    30 |       0 | %now-2d |

Scenario: The investment club issues dividends
  Given these "txs":
  | xid | created | amount | payer | payee | purpose    | flags |*
  |   1 | %now-3d |     10 | .ZZA  | .ZZI | investment | stake |
  |   2 | %now-1d |     70 | .ZZB  | .ZZI | investment | stake |
  And balances:
  | uid  | balance |*
  | .ZZI |    5000 |
  |  cgf |   -5320 |
  And these "r_stakes":
  | stakeid | uid  | clubid | stake | request | joined  |*
  |       1 | .ZZA | .ZZI   |    30 |     -20 | %now-2d |
  |       2 | .ZZB | .ZZI   |    70 |     -40 | %now-2d |
  And these "r_investments":
  | vestid | coid | clubid | proposedBy | investment | return | types | terms | assets | offering | price | character | strength | web | history | soundness | reserve |*
  |      1 | .ZZC | .ZZI   | .ZZB       | improve it | 0.046  | D     | Terms | 951000 |    10000 |    10 | trusty    |       75 |  60 |      80 |        90 |       0 |
  And these "r_shares":
  | shid | vestid | shares | pending | when    |*
  |    1 |      1 |    100 |       0 | %now-2m |
  |    2 |      1 |    -40 |       0 | %now-1m |
  When member "I:B" visits page "invest/cashout"
  Then we show "Handle Requests to Cash Out Investments" with:
  | Available Funds | $5,000 |
  | Total Requests | $60 (2) |
  | | Cash Them Out |
  And without:
  | Method |
  
  When member "I:B" visits page "invest/dividends"
  Then we show "Issue Dividends" with:
  | Available for Dividends | $5,500 |
  | | (10.0% is reserved for Common Good) |
  | Total Dividends to Issue | $5500 |
  | Issue Dividends | |
  
  When member "I:B" completes form "invest/dividends" with values:
  | amount | avail |*
  | $5,000 | 5500  |
  Then we say "status": "dividends paid" with subs:
  | got | count | sum    | reCount | reSum  |*
  |   2 |     2 | $4,500 |       1 | $3,150 |
  And these "txs":
  | xid | created | amount | payer | payee | purpose                    | flags |*
  |   3 | %now    |    500 | .ZZI  |  cgf | community dividend         |       |
  |   4 | %now    |   1350 | .ZZI  | .ZZA | dividend                   |       |
  |   5 | %now    |   3150 | .ZZI  | .ZZB | dividend                   |       |
  |   6 | %now    |   3150 | .ZZB  | .ZZI | re-investment of dividends | stake |
  And these "r_stakes":
  | stakeid | uid  | clubid | stake | request | joined  |*
  |       1 | .ZZA | .ZZI   |    30 |     -20 | %now-2d |
  |       2 | .ZZB | .ZZI   |  3180 |       0 | %now-2d |
  And balances:
  | uid  | balance |*
  |  cgf |   -4820 |
  | .ZZA |    1540 |
  | .ZZB |     130 |
  | .ZZI |    3150 |
  
#-------------------------------------------------------------------------------------------

# Variations for loans (as opposed to equity investments)

Scenario: Members rate a proposed loan
  Given these "r_stakes":
  | stakeid | uid  | clubid | stake | joined  |*
  |       1 | .ZZA | .ZZI   |    33 | %now-2d |
  And these "r_investments":
  | vestid | coid | clubid | proposedBy | investment | return | types | terms | assets | offering | price | character | strength | web | history | soundness | reserve |*
  |      1 | .ZZC | .ZZI   | .ZZB       | improve it | 0.046  | I     | Terms | 951000 |    10000 |     1 | trusty    |       75 |  60 |      80 |        90 |       0 |
  When member ".ZZA" visits page "invest/rate/vestid=1&clubqid=NEWZZI"
  Then we show "View or Rate Investment #1" with:
  | Numeric ratings are on a scale of | 0-100 |
  | Company | Our Pub |
  | City | Cton |
  | Service area | |
  | Founded | %mdY-1d |
  | Annual gross | $0 |
  | Co description | |
  | Project/Purpose | improve it |
  | Target | $10,000 |
  | Interest Rate | 4.6% |
  | Terms | Terms |
  | Company assets | $951,000 |
  | Owner character | trusty |
  | Financial strength | 75 |
  | Web presence | 60 |
  | Repayment history | 80 |
  | Overall soundness | 90 |
  | Common Goodness | Rate this investment's benefit |
  | Your patronage | |
  | Comments | |
  | | Rate it |

  Given these "r_ratings":
  | ratingid | vestid | uid  | good | comment | patronage |*
  |        1 |      1 | .ZZD |   40 | yay!    |        80 |
  When member ".ZZA" completes form "invest/rate/vestid=1&clubqid=NEWZZI" with values:
  | good | patronage | comment |*
  |   20 |        50 | do it!  |
  Then we say "status": "rating successful" with subs:
  | num |*
  |   1 |
  And these "r_ratings":
  | ratingid | vestid | uid  | good | comment | patronage |*
  |        2 |      1 | .ZZA |   20 | do it!  |        50 |
  And we show "Club Investments" with:
  | Investment          | Type   | Return | Value | Good |
  | Our Pub: improve it | Loan   |  4.6% |       |   30 |
  
  When member ".ZZA" visits page "invest/rate/vestid=1&clubqid=NEWZZI"
  Then we show "View or Rate Investment #1" with:
  | Company | Our Pub |
  | Your rating | 20 |
  | Your patronage | 50 |
  | Your comment | do it! |
  And without:
  | | Rate it |

Scenario: The club makes a loan
  Given these "r_stakes":
  | stakeid | uid  | clubid | stake | joined  |*
  |       1 | .ZZA | .ZZI   |   200 | %now-2d |
  And member ".ZZA" completes form "invest" with values:
  | op  | amount |*
  | buy |    200 |
  And these "r_investments":
  | vestid | coid | clubid | proposedBy | investment | return | types | terms | assets | offering | price | character | strength | web | history | soundness | reserve |*
  |      1 | .ZZC | .ZZI   | .ZZB       | improve it | 0.046  | I     | Terms | 951000 |    10000 |     1 | trusty    |       75 |  60 |      80 |        90 |       0 |

  When member "I:B" visits page "invest/rate/vestid=1&clubqid=NEWZZI"
  Then we show "Investment #1" with:
  | Numeric ratings are on a scale of | 0-100 |
  | Common Goodness | |
  | Patronage | |
  | Company | Our Pub |
  | City | Cton |
  | Service area | |
  | Founded | %mdY-1d |
  | Annual gross | $0 |
  | Co description | |
  | Project/Purpose | improve it |
  | Target | $10,000 |
  | Interest Rate | 4.6% |
  | Terms | Terms |
  | Company assets | $951,000 |
  | Owner character | trusty |
  | Financial strength | 75 |
  | Web presence | 60 |
  | Repayment history | 80 |
  | Overall soundness | 90 |
  | Comments | |
  And without:
  | | Rate this investment's benefit |
  | Your patronage | |
  | | Rate it |
  
  When member "I:B" visits page "invest/buy-or-sell/vestid=1"
  Then we show "Buy or Sell Investment #1" with:
  | increase or decrease the club's loan to Our Pub | |
  | Current Loan | $0 |
  | Amount to Lend | |
  | | Lend More |
  And without:
  | | Request Repayment |
  
  When member "I:B" completes form "invest/buy-or-sell/vestid=1" with values:
  | op  | shares |*
  | buy |    100 |
  Then we say "status": "report tx|loaned" with subs:
  | did  | otherName | amount | co      |*
  | paid | Our Pub   |   $100 | Our Pub |
  And these "r_shares":
  | shid | vestid | shares | pending | when | sold |*
  |    1 |      1 |    100 |       0 | %now |      |
  And we show "Club Investments" with:
  | Investment          | Type | Return | Value |
  | Our Pub: improve it | Loan |  4.6% | 100   |
  
  When member "I:B" visits page "invest/buy-or-sell/vestid=1"
  Then we show "Buy or Sell Investment #1" with:
  | increase or decrease the club's loan to Our Pub | |
  | Current Loan | $100 |
  | Pending Repayment Request | 0 |
  | Lend or Reclaim | |
  | | Lend More |
  | | Request Repayment |

Scenario: The club sells shares
  Given these "r_stakes":
  | stakeid | uid  | clubid | stake | joined  |*
  |       1 | .ZZA | .ZZI   |   200 | %now-2d |
  And member ".ZZA" completes form "invest" with values:
  | op  | amount |*
  | buy |    200 |
  And these "r_investments":
  | vestid | coid | clubid | proposedBy | investment | return | types | terms | assets | offering | price | character | strength | web | history | soundness | reserve |*
  |      1 | .ZZC | .ZZI   | .ZZB       | improve it | 0.046  | I     | Terms | 951000 |    10000 |     1 | trusty    |       75 |  60 |      80 |        90 |       0 |
  And these "r_shares":
  | shid | vestid | shares | pending | when |*
  |    1 |      1 |    100 |       0 | %now |
  
  # now request sale of 4 of the 10 shares
  When member "I:B" completes form "invest/buy-or-sell/vestid=1" with values:
  | op   | shares |*
  | sell |     40 |
  Then we say "status": "report tx|balance unchanged|repayment request pending" with subs:
  | did     | otherName | amount | co      |*
  | charged | Our Pub   |    $40 | Our Pub |
  And these "tx_requests":
  | nvid | payer | payee | amount | purpose        | flags      |*
  |    1 | .ZZC  | .ZZI  |     40 | loan repayment | investment |
  And these "r_shares":
  | shid | vestid | shares | pending | when | sold |*
  |    1 |      1 |    100 |       0 | %now |      |
  |    2 |      1 |      0 |     -40 | %now |      |
  When member "I:B" visits page "invest/buy-or-sell/vestid=1"
  Then we show "Buy or Sell Investment #1" with:
  | increase or decrease the club's loan to Our Pub | |
  | Current Loan | $100 |
  | Pending Repayment Request | $40 |
  | Lend or Reclaim | |
  | | Lend More |
  | | Request Repayment |  

  # finish sale
  When member "C:A" completes form "handle-invoice/nvid=1&code=TESTDOCODE" with values:
  | op  |*
  | pay |
  Then these "r_shares":
  | shid | vestid | shares | pending | when | sold |*
  |    1 |      1 |    100 |       0 | %now |      |
  |    2 |      1 |    -40 |       0 | %now |      |
  When member ".ZZA" visits page "invest/list/clubqid=NEWZZI"
  Then we show "Club Investments" with:
  | Investment          | Type | Return | Value |
  | Our Pub: improve it | Loan |  4.6% | 60    |
  When member "I:B" visits page "invest/buy-or-sell/vestid=1"
  Then we show "Buy or Sell Investment #1" with:
  | increase or decrease the club's loan to Our Pub | |
  | Current Loan | $60 |
  | Pending Repayment Request | 0 |
  | Lend or Reclaim | |
  | | Lend More |
  | | Request Repayment | 
  
Scenario: The club sells its remaining shares in an investment
  Given these "r_stakes":
  | stakeid | uid  | clubid | stake | joined  |*
  |       1 | .ZZA | .ZZI   |   200 | %now-2d |
  And member ".ZZA" completes form "invest" with values:
  | op  | amount |*
  | buy |    200 |
  And these "r_investments":
  | vestid | coid | clubid | proposedBy | investment | return | types | terms | assets | offering | price | character | strength | web | history | soundness | reserve |*
  |      1 | .ZZC | .ZZI   | .ZZB       | improve it | 0.046  | I     | Terms | 951000 |    10000 |     1 | trusty    |       75 |  60 |      80 |        90 |       0 |
  And these "r_shares":
  | shid | vestid | shares | pending | when |*
  |    1 |      1 |    100 |       0 | %now |
  |    2 |      1 |    -40 |       0 | %now |

  # sell the last 6 shares
  When member "I:B" completes form "invest/buy-or-sell/vestid=1" with values:
  | op   | shares |*
  | sell |     60 |
  Then we say "status": "report tx|balance unchanged|repayment request pending" with subs:
  | did     | otherName | amount | co      |*
  | charged | Our Pub   |    $60 | Our Pub |
  And these "tx_requests":
  | nvid | payer | payee | amount | purpose        | flags      |*
  |    1 | .ZZC  | .ZZI  |     60 | loan repayment | investment |  
  When member "C:A" completes form "handle-invoice/nvid=1&code=TESTDOCODE" with values:
  | op  |*
  | pay |
  Then these "r_shares":
  | shid | vestid | shares | pending | when | sold |*
  |    1 |      1 |    100 |       0 | %now | %now |
  |    2 |      1 |    -40 |       0 | %now | %now |
  |    3 |      1 |    -60 |       0 | %now | %now | 