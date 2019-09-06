Feature: Queries
AS a member
I WANT to run queries
SO I can see what's going on in my finances and in the community's economy.

Setup:
  Given members:
  | uid  | fullName   | floor | acctType    | flags                | created    | risks   |*
  | .ZZA | Abe One    | -100  | personal    | ok,roundup,confirmed,bankOk | %today-15m | hasBank |
  | .ZZB | Bea Two    | -200  | personal    | ok,admin             | %today-15m |         |
  | .ZZC | Corner Pub | -300  | corporation | ok,co                | %today-15m |         |
  And relations:
  | main | agent | permission |*
  | .ZZA | .ZZB  | buy        |
  | .ZZB | .ZZA  | read       |
  | .ZZC | .ZZB  | buy        |
  | .ZZC | .ZZA  | sell       |
  And usd transfers:
  | txid | payee | amount | created    | completed  | deposit    |*
  |   11 |  .ZZA |   1000 | %today-13m | %today-13m | %today-13m |
  |   12 |  .ZZB |   2000 | %today-13m | %today-13m | %today-13m |
  |   13 |  .ZZC |   3000 | %today-13m | %today-13m | %today-13m |
  |   14 |  .ZZA |     11 | %today-3d  |         0  | %today-13m |
  |   15 |  .ZZA |    -22 | %today-5d  | %today-5d  |          0 |
  |   16 |  .ZZA |    -33 | %today-5d  | %today-5d  |          0 |
  # The usd transfers create same-numbered transactions
  And balances:
  | uid  | balance |*
  | .ZZA |     945 |
  | .ZZB |    2000 |
  | .ZZC |    3000 |
  Given transactions: 
  | xid | created   | amount | from | to   | purpose | taking | payerTid | payeeTid |*
  |  44 | %today-5m |     10 | .ZZB | .ZZA | cash E  | 0      |       24 |       34 |
  |  45 | %today-4m |   1100 | .ZZC | .ZZA | usd F   | 1      |       25 |       35 |
  |  46 | %today-3m |    240 | .ZZA | .ZZB | what G  | 0      |       26 |       36 |
  |  47 | %today-2w |     50 | .ZZB | .ZZC | cash P  | 0      |       27 |       37 |
  |  48 | %today-1w |    120 | .ZZA | .ZZC | this Q  | 1      |       28 |       38 |
  |  49 | %today-6d |    100 | .ZZA | .ZZB | cash V  | 0      |       29 |       39 |
  Then balances:
  | uid  | balance |*
  | .ZZA |    1595 |
  | .ZZB |    2280 |
  | .ZZC |    2070 |

Scenario: A member visits the Community Data page
  When member ".ZZA" visits page "community/data"
  Then we show "Community and Money Data" with:
  | Positive and Negative Balance Totals |
  | Company and Member Balances and Credit Lines |
  | Actual Donations to Common Good and Community |
  | Expected Member Donations to CG and Community |
  | Expected Company Donations to CG and Community |
  And without:
  | Food Fund Percentage Contributions |
  | Most Trusted Members |
  | Individuals Paid By A Company |
  | Transaction Totals to Date |
  | Monthly Business Income (30 day totals) |
  | Where Do Our Members Hear About Common Good |
  
Scenario: An admin visits the Community Data page
  When member ".ZZB" visits page "community/data"
  Then we show "Community and Money Data" with:
  | Food Fund Percentage Contributions |
  | Most Trusted Members |
  | Individuals Paid By A Company |
  | Transaction Totals to Date |
  | Monthly Business Income (30 day totals) |
  | Positive and Negative Balance Totals |
  | Company and Member Balances and Credit Lines |
  | Actual Donations to Common Good and Community |
  | Expected Member Donations to CG and Community |
  | Expected Company Donations to CG and Community |
  | Where Do Our Members Hear About Common Good |
  
Scenario: An admin runs a query about Food Fund
  When member ".ZZB" runs query "Food Fund Percentage Contributions"
  Then we show "Food Fund Percentage Contributions" with:
  | participants | monthly |
  |            0 |         |
  
Scenario: An admin runs a query about Trusted Members
  When member ".ZZB" runs query "Most Trusted Members"
  Then we show "" with:
  | fullName | email         | phone | proxiedFor | trust |
  | Abe One  | a@example.com |       |          0 |     0 |
  | Bea Two  | b@example.com |       |          0 |     0 |

Scenario: A member runs a query about Employees
  When member ".ZZB" runs query "Individuals Paid By A Company"
  Then we show "" with:
  | company    | pays    | relation |
  | Corner Pub | Abe One |          |
  | Corner Pub | Bea Two |          |
  
Scenario: A member runs a query about Transaction Totals
  When member ".ZZB" runs query "Transaction Totals to Date"
  Then we show "" with:
  | txCount | txDollars | inPersonDollars | ccFeesSaved |
  | 12      |  7,675.00 |        3,170.00 |   53.890000 |

Scenario: A member runs a query about Business Income
  When member ".ZZB" runs query "Monthly Business Income (30 day totals)"
  Then we show "" with:
  | company    | total $ | count | total $ 6mos ago | count6 | total $ 12mos ago | count12 |
  | Corner Pub |  120.00 |     1 |                  |        |                   |         |

Scenario: A member runs a query about Positive and Negative
  When member ".ZZA" runs query "Positive and Negative Balance Totals"
  Then we show "" with:
  | community | negativeBalanceCount | negativeBalanceTotal | positiveBalanceCount | positiveBalanceTotal |
  | Common Good Western Mass | 0 | 0.00 | 5 | 5,945.00 |

Scenario: A member runs a query about Balances
  When member ".ZZA" runs query "Company and Member Balances and Credit Lines"
  Then we show "" with:
  | community                | memBalances | memCredit | memTargetBalances | coBalances | coCredit | coTargetBalances |
  | Common Good Western Mass |    3,875.00 |    300.00 |              0.00 |   2,070.00 |   300.00 |             0.00 |

Scenario: A member runs a query about Actual Donations
  When member ".ZZA" runs query "Actual Donations to Common Good and Community"
  Then we show "" with:
  | data set is empty |

Scenario: A member runs a query about Expected Member Donations
  When member ".ZZA" runs query "Expected Member Donations to CG and Community"
  Then we show "" with:
  | community                | members | yearly$ | roundupy | crumby | avgCrumbs |
  | Common Good Western Mass |       2 |         |        1 |      0 |           |

Scenario: A member runs a query about Expected Company Donations
  When member ".ZZA" runs query "Expected Company Donations to CG and Community"
  Then we show "" with:
  | community                | companies | yearly$ | roundupy | crumby | avgCrumbs |
  | Common Good Western Mass |         3 |         |        0 |      0 |           |

Scenario: A member runs a query about Whence
  When member ".ZZB" runs query "Where Do Our Members Hear About Common Good"
  Then we show "" with:
  | origin    | member | stuck | canceled | avgYearlyGift$ | avgRoundup | invited |
  | * Invited |      3 |     0 |        0 |           0.00 |     0.3333 |         |
  | * Other   |      7 |     4 |        0 |           0.00 |     0.0000 |         |