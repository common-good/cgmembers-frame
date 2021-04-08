Feature: A company scans a member card with a standard QR app or with CGPay for iOS
AS a member company
I WANT to charge a member company or individual by scanning their card on any device
SO I can avoid having multiple devices in my pocket or at the checkout counter

Setup:
  Given members:
  | uid  | fullName | email | flags                       | zip   | floor | cardCode | city | state | pass |*
  | .ZZA | Abe One  | a@    | member,ok,confirmed,debt    | 01001 |  -100 | Aa1      | Aville | AL  | 123  |
  | .ZZB | Bea Two  | b@    | member,ok,confirmed,debt    | 01001 |  -100 | Bb2      | Bville | DC  | 123  |
  | .ZZC | Our Pub  | c@    | member,ok,co,confirmed,debt | 01003 |  -100 |          | Cville | CA  |      |
  And member ".ZZA" has picture "picture1"

Scenario: A member scans an individual card, with no scanner set, not signed in
  Given cookie "scanner" is ""
  When member "?" visits "card/6VM/KDJIAa1"
  Then we first "signin" then "url=card/6VM/KDJIAa1"
  And we show
  | Welcome to %PROJECT |
  
  When member "?" completes "signin/then=TESTCODE" with:
  | name   | pass |*
  | NEWZZB | 123  |
  Then cookie "scanner" is "NEWZZB"
  And we show
  | You: Bea Two ||
  | picture1     ||
  | Abe One      ||
  | Aville, AL   ||
  | Amount       ||
  | For          ||
  | Charge       ||
  And with options:
  | Other |

Scenario: A member scans an individual card, with no scanner set, signed in, no relations
  Given cryptcookie "qid" is "NEWZZB"
  And cookie "trust" is "1"
  And cookie "scanner" is ""

  When member ".ZZB" visits "card/6VM/KDJIAa1"
  Then cookie "scanner" is "NEWZZB"
  And we show
  | You: Bea Two ||
  | picture1     ||
  | Abe One      ||
  | Aville, AL   ||
  | Amount       ||
  | For          ||
  | Charge       ||
  And with options:
  | Other |

Scenario: A member scans an individual card, with no scanner set, signed in, with relations choices
  Given relations:
  | main | other | permission | otherNum |*
  | .ZZC | .ZZB  | manage     | 1        |
  And members have:
  | uid  | selling   |*
  | .ZZC | groceries |
  And cryptcookie "qid" is "NEWZZB"
  And cookie "trust" is "1"
  And cookie "scanner" is ""

  When member ".ZZB" visits "card/6VM/KDJIAa1"
  Then we show "Scan From Which Account?" with:
  | Bea Two |
  | Our Pub |

  When member ".ZZB" completes "card/6VM/KDJIAa1" with:
  | mode   | tail        | account |*
  | choose | 6VM/KDJIAa1 | .ZZC    |
  Then cookie "scanner" is "NEWZZC"
  And we redirect to "card/6VM/KDJIAa1"
  And we show
  | You: Our Pub ||
  | picture1     ||
  | Abe One      ||
  | Aville, AL   ||
  | Amount       ||
  | For          ||
  | Charge       ||
  And with options:
  | groceries |

Scenario: A member scans an individual card, with scanner set, not signed in, no relations, trusted
  Given members have:
  | uid  | selling   |*
  | .ZZC | groceries |
  And cookie "trust" is "1"
  And cookie "scanner" is "NEWZZC"

  When member "?" visits "card/6VM/KDJIAa1"
  Then we show
  | You: Our Pub ||
  | picture1     ||
  | Abe One      ||
  | Aville, AL   ||
  | Amount       ||
  | For          ||
  | Charge       ||
  | Pay          ||
  And with options:
  | groceries |  

Scenario: A member scans an individual card, with scanner set, not signed in, no relations, not trusted
  Given members have:
  | uid  | selling   |*
  | .ZZC | groceries |
  And cookie "trust" is ""
  And cookie "scanner" is "NEWZZC"

  When member "?" visits "card/6VM/KDJIAa1"
  Then we show
  | You: Our Pub ||
  | picture1     ||
  | Abe One      ||
  | Aville, AL   ||
  | Amount       ||
  | For          ||
  | Charge       ||
  And with options:
  | groceries |
  And without:
  | Pay          ||

Scenario: A member scans an individual card, with scanner set, signed in, no relations
  Given members have:
  | uid  | selling   |*
  | .ZZC | groceries |
  And cookie "trust" is ""
  And cookie "scanner" is "NEWZZC"

  When member ".ZZB" visits "card/6VM/KDJIAa1"
  Then we show
  | You: Bea Two ||
  | picture1     ||
  | Abe One      ||
  | Aville, AL   ||
  | Amount       ||
  | For          ||
  | Charge       ||
  And with options:
  | Other |

Scenario: a member card is charged, with scanner set, not signed in
  Given members have:
  | uid  | selling   |*
  | .ZZC | groceries |
  And cryptcookie "qid" is "NEWZZB"
  And cookie "scanner" is "NEWZZC"
  And cookie "trust" is "1"
  
  When member "?" completes "card/6vm/KDJIAa1" with:
  | op     | amount | desc      |*
  | charge | 10     | groceries |
  Then we say "status": "report tx" with subs:
  | did     | otherName | amount |*
  | charged | Abe One   | $10    |
  And transactions:
  | xid | created | amount | payer | payee | purpose   | taking |*
  |   1 | %today  |     10 | .ZZA  | .ZZC  | groceries | 1      |

Scenario: a member card is paid, with scanner set, not signed in
  Given members have:
  | uid  | selling   |*
  | .ZZC | groceries |
  And cryptcookie "qid" is "NEWZZB"
  And cookie "scanner" is "NEWZZC"
  And cookie "trust" is "1"
  
  When member "?" completes "card/6vm/KDJIAa1" with:
  | op     | amount | desc      |*
  | pay    | 10     | groceries |
  Then we say "status": "report tx" with subs:
  | did     | otherName | amount |*
  | paid    | Abe One   | $10    |
  And transactions:
  | xid | created | amount | payer | payee | purpose   | taking |*
  |   1 | %today  |     10 | .ZZC  | .ZZA  | groceries | 0      |
