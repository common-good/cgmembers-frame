Feature: A company signs up to try Common Good using a short form
AS an owner or manager of a company
I WANT to open a Common Good account
SO I can be part of the Common Good Economy

# phone 1 means +1 413.772.0001

Setup:
  Given members:
  | uid | fullName  | phone | email | city  | state | zip   | floor | flags   | pass      | helper |*
  | .ZZA | Abe One  |     1 | a@    | Atown | AK    | 01000 |     0 | member  | %whatever | cgf    |
  | .ZZC | Our Pub  |     3 | c@    | Ctown | CA    | 03000 |     0 | co      |           | .ZZA   |
  | .ZZZ | Zeta Zot |    26 | z@    | Ztown | MS    | 09000 |     0 | co      | zpass     | cgf    |
  And these "u_relations":
  | main | other | permission | otherNum |*
  | .ZZC | .ZZA  | manage     | 1        |
  And member is logged out

Scenario: A company tries to sign up directly
  When member "?" visits page "signup-co"
  Then we show "Open a Company Account" with:
  | open a personal account |

Scenario: Someone wants to open a company account
  When member ".ZZA" visits page "signup-co/relate=1"
  Then we show "Open a Company Account" with:
  | Account Type  | |
  | Company       | |
  | Legal Name    | |
  | Email         | |
  | Postal Code   | |
  | Company Phone | |
  | Selling       | |
  | Sell Credit   | |
  | Founded       | |
  | Federal ID    | |

Scenario: A company signs up
  Given next codes are "zot zot WHATEVER"
  When member ".ZZA" completes form "signup-co/relate=1" with values:
  | fullName  | New Co       |**
  | legalName |              |
  | federalId | 04-3849283   |
  | founded   | %mdY-3y      |
  | zip       | 01004        |
  | phone     | 413-253-0004 |
  | email     | d@           |
  | selling   | fish         |
  | sellCG    | 1            |
  | coType    | LLC          |
  Then members:
  | uid       | .AAA         |**
  | fullName  | New Co       |
  | legalName | New Co       |
  | federalId | 043849283    |
  | founded   | %daystart-3y |
  | zip       | 01004        |
  | phone     | +14132530004 |
  | email     | d@           |
  | selling   | fish         |
  | coType    | LLC          |
#  | coFlags   | sellCG       |
  And these "u_relations":
  | main | other | permission | otherNum |*
  | .AAA | .ZZA  | manage     | 1        |
  And we email "verify-co" to member "d@" with subs:
  | fullName | qid    | site      | code     | pwMsg      |*
  | New Co   | NEWAAA | %BASE_URL | WHATEVER | co nonpass |
  And we show "Connect a Checking Account"
  And we say "status": "info saved|step completed"
  And member ".AAA" steps left "contact backing photo donate company crumbs verifyemail"
  # step "fund" is optional, so it is marked "done for now" even if skipped
  And members have:
  | uid  | signed | signedBy |*
  | .AAA | %today | Abe One  |

  Given member is logged out
  When member "?" visits page "settings/verifyemail/qid=NEWAAA-A&verify=1&code=WHATEVER"
  Then we show "Welcome to %PROJECT" with:
  | Account |
  | Password |
  And we say "status": "verified email"
  And we say "status": "info saved|step completed"
  And we say "status": "continue co setup"
  And member ".AAA" steps left "contact backing photo donate company crumbs"

Scenario: A company verifies email while signed in
  Given member ".ZZC" has "co" steps done: "signup fund contact backing photo donate company crumbs"
  And members have:
  | uid  | flags  |*
  | .ZZA | ok     |
  And member ".ZZC" one-time password is "WHATEVER"
  When member "C:A" visits page "settings/verifyemail/qid=NEWZZC-A&verify=1&code=WHATEVER"
  Then we show "You: Our Pub"
  And we say "status": "verified email"
  And we say "status": "info saved|success|co ok|pioneer thanks"
  And member ".ZZC" steps left ""
  And members have:
  | uid  | flags             |*
  | .ZZC | co,ok,member,ided |
  
Scenario: A company supplies company information
  Given member ".ZZC" has "co" steps done: "signup verifyemail fund contact backing photo donate"
  And members have:
  | uid  | website   | selling |*
  | .ZZC | ourpub.co | drinks  |
  When member "C:A" visits page "settings/company"
  Then we show "Company Settings" with:
  | Shortcuts       | |
  | Name            | Our Pub |
  | Private         | |
  | Categories      | |
  | Selling         | drinks |
  | Short Desc      | |
  | Employees       | |
  | Annual Gross    | |
  | Website         | ourpub.co |
  | Founded         | |
  | Describe        | |
  | Nudge Every     | |

  When member "C:A" completes form "settings/company" with values:
  | fullName    | Our Pub |**
  | private     | 1 |
  | categories  | 0=>%CAT_FOOD, 1=>%CAT_RETAIL |
  | selling     | ale |
  | shortDesc   | bar |
  | employees   | 3 |
  | gross       | $250,000 |
  | founded     | %mdY-9y |
  | website     | example.com |
  | description | really good ale |
  | staleNudge  | 8 |
  Then members have:
  | uid         | .ZZC |**
  | fullName    | Our Pub  |
  | selling     | ale |
  | shortDesc   | bar |
  | employees   | 3 |
  | gross       | 250000 |
  | founded     | %daystart-9y |
  | website     | example.com |
  | description | really good ale |
  | staleNudge  | 8 |
  # should be | coFlags     | private | but not until everyone's using our node.js app
  And we show "Share When You Receive" with:
  | Crumbs |
  And we say "status": "info saved|step completed"
  And member ".ZZC" steps left "crumbs"

Scenario: A company supplies crumbs choices
  Given member ".ZZC" has "co" steps done: "signup verifyemail fund contact backing photo donate company"
  And members have:
  | uid  | flags     |*
  | .ZZA | member ok |
  When member "C:A" completes form "community/crumbs" with values:
  | crumbs | 3 |**
  Then members have:
  | uid  | crumbs |*
  | .ZZC | .03    |

#  And we show "Get Your Customers Signed Up"
#  And we say "status": "info saved|step completed"
#  And member ".ZZC" steps left "discount"

#Scenario: A company account manager creates a discount
#  Given member ".ZZC" has "co" steps done: "signup verifyemail fund contact backing photo donate company crumbs"
#  And members have:
#  | uid  | task   |*
#  | .ZZC | co     |
#  And members have:
#  | uid  | flags     |*
#  | .ZZA | ok,member |
#  When member "C:A" completes form "community/discount" with values:
#  | amount | minimum | start | end     | useMax | type     |*
#  |     20 |     120 | %mdY  | %mdY+3m |      3 | discount |
#  Then these "coupons":
#  | coupid | fromId | amount | useMax | flags | start      | end                         |*
#  |      1 |   .ZZC |     20 |      3 |       | %daystart  | %(%daystart+3m+%DAY_SECS-1) |
#  And we say "status": "Your discount was created successfully."
#  And we tell ".ZZC" CO "New Coupons!" with subs:
#  | quid      | .ZZC |**
#  | type      | discount |
#  | amount    | 20 |
#  | minimum   | 120 |
#  | useMax    | 3 |
#  | purpose   | on your purchase of $120 or more |
#  | start     | %daystart |
#  | end       | %(%daystart+3m+%DAY_SECS-1) |
#  | automatic | 1 |
#  | company   | Our Pub |
#  | gift      | |
 # | forOnly   | |

  And we show "You: Our Pub" with:
  | Balance: | $0 |
  And without:
  | Finish |
  And we say "status": "info saved|success|co ok|pioneer thanks"
  And member ".ZZC" steps left ""
#  And we tell ".ZZC" CO "New Member (Our Pub)" with subs:
#  | quid | status |*
#  | .ZZC | member |

Scenario: An unverified company agent completes a company account
  Given member ".ZZC" has "co" steps done: "signup verifyemail fund contact backing photo donate company"
  And members have:
  | uid  | flags  |*
  | .ZZA |        |
  When member "C:A" completes form "community/crumbs" with values:
  | crumbs | 3 |**
  Then we say "status": "info saved|success"
  And we say "status": "co tentative|pioneer thanks"
  And member ".ZZC" steps left ""
