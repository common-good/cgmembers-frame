Feature: Update Card
AS a non-member who donates by credit card to an organization through Common Good
I WANT to replace the card used for my recurring donations
SO my donations continue when my card changes

Setup:
  Given members:
  | uid  | fullName | flags              |*
  | .ZZA | Abe One  | ok,confirmed,admin |
  | .ZZC | Our Pub  | ok,confirmed,co    |
  | .ZZD | Dee Co   | ok,confirmed,co    |
  And these "admins":
  | uid  | vKeyE     | can                |*
  | .ZZA | DEV_VKEYE | v,panel,mutualAid  |
  And these "people":
  | pid | fullName | email | stripeCid |*
  | 1   | Zee Zot  | z@    | cus1      |
  | 2   | Yoyo Yot | y@    | cus2      |
  | 3   | Xan Xot  | x@    |           |
  And these "tx_timed":
  | id | action   | from         | to   | amount | purpose  | payerType   | payer | payeeType    | period | periods | duration | start     | end      | stripeId | failed  |*
  | 1  | %ACT_PAY | %MATCH_PAYER | .ZZC | 10     | donation | %REF_PERSON | 1     | %REF_ANYBODY | month  | 1       | once     | %now-3m   |          | seti1    | %now-2d |
  | 2  | %ACT_PAY | %MATCH_PAYER | .ZZD | 50     | donation | %REF_PERSON | 1     | %REF_ANYBODY | year   | 1       | once     | %now-3m   | %now+1y  | seti2    | %NUL    |
  | 3  | %ACT_PAY | %MATCH_PAYER | .ZZD | 20     | donation | %REF_PERSON | 1     | %REF_ANYBODY | month  | 1       | once     | %now-3m   | %now-1d  | seti3    | %NUL    |
  | 4  | %ACT_PAY | %MATCH_PAYER | .ZZC | 30     | donation | %REF_PERSON | 2     | %REF_ANYBODY | month  | 1       | once     | %now-3m   |          | seti4    | %now-2d |

# Rule: staff get a link only for a donor with recurring card donations
Scenario: An admin looks at a card donor
  When member ".ZZA" visits page "sadmin/people/pid=1"
  Then we show "People" with:
  | Copy card-update link |

Scenario: An admin looks at a non-member who does not donate by card
  When member ".ZZA" visits page "sadmin/people/pid=3"
  Then we show "People" with:
  | Stripe Cust ID |
  And without:
  | Copy card-update link |

# Rule: the link shows the donor's current recurring card donations, including paused ones
Scenario: A donor opens a card-update link
  Given var "code" encrypts:
  | pid | cid  | expires  |*
  | 1   | cus1 | %now+25d |
  When someone visits "update-card/code=%code"
  Then we show "Update Your Card" with:
  | Hello Zee Zot |
  | $10 monthly to Our Pub (paused) |
  | $50 yearly to Dee Co |
  | Card: |
  | Update |
  And without:
  | $20 |
  | $30 |

# Rule: the link works only for its donor, only until it expires
Scenario: A donor opens an expired card-update link
  Given var "code" encrypts:
  | pid | cid  | expires |*
  | 1   | cus1 | %now-1d |
  When someone visits "update-card/code=%code"
  Then we say "error": "That link is invalid or has expired. Please ask us for a new one."

Scenario: A card-update link names a different Stripe customer
  Given var "code" encrypts:
  | pid | cid  | expires  |*
  | 1   | cus2 | %now+25d |
  When someone visits "update-card/code=%code"
  Then we say "error": "That link is invalid or has expired. Please ask us for a new one."

# Rule: the new card replaces the old one in every current recurring card donation and resumes the paused ones
Scenario: A donor saves a new card
  Given var "code" encrypts:
  | pid | cid  | expires  |*
  | 1   | cus1 | %now+25d |
  When someone submits "update-card/code=%code" with:
  | code  | setupId |*
  | %code | setiNew |
  Then these "tx_timed":
  | id | stripeId | failed  |*
  | 1  | setiNew  |         |
  | 2  | setiNew  |         |
  | 3  | seti3    |         |
  | 4  | seti4    | %now-2d |
  And we tell admin "Donor updated card" with subs:
  | pid | fullName | recurs |*
  | 1   | Zee Zot  | 2      |
  And we say "status": "Thank you! Your new card will be used for your future donations."
