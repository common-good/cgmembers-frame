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
  | qid  | pass |*
  | .ZZB | 123  |
  Then cookie "scanner" is "NEWZZB"
  And we show
  | You: Bea Two ||
  | picture1     ||
  | Abe One      ||
  | Aville, AL   ||
  | For          ||
  | Amount       ||
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
  | For          ||
  | Amount       ||
  | Charge       ||
  And with options:
  | Other |

Scenario: A member scans an individual card, with no scanner set, signed in, with relations choices
  Given these "u_relations":
  | main | other | permission | otherNum |*
  | .ZZC | .ZZB  | manage     | 1        |
  And members have:
  | uid  | selling   |*
  | .ZZC | groceries |
  And cryptcookie "qid" is "NEWZZB"
  And cookie "trust" is "1"
  And cookie "scanner" is ""
  And cookie "box" is "abcd"
  And these "r_boxes":
  | id | channel | code | boxnum | uid  |*
  | 2  | %TX_WEB | abcd | 12345  | .ZZC |

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
  | For          ||
  | Amount       ||
  | Charge       ||
  And with options:
  | groceries |

Scenario: A member scans an individual card, with scanner set, not signed in, no relations, trusted
  Given members have:
  | uid  | selling   |*
  | .ZZC | groceries |
  And cookie "trust" is "1"
  And cookie "scanner" is "NEWZZC"
  And cookie "box" is "abcd"
  And these "r_boxes":
  | id | channel | code | boxnum | uid  |*
  | 2  | %TX_WEB | abcd | 12345  | .ZZC |

  When member "?" visits "card/6VM/KDJIAa1"
  Then we show
  | You: Our Pub ||
  | picture1     ||
  | Abe One      ||
  | Aville, AL   ||
  | For          ||
  | Amount       ||
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
  And cookie "box" is "abcd"
  And these "r_boxes":
  | id | channel | code | boxnum | uid  |*
  | 2  | %TX_WEB | abcd | 12345  | .ZZC |

  When member "?" visits "card/6VM/KDJIAa1"
  Then we show
  | You: Our Pub ||
  | picture1     ||
  | Abe One      ||
  | Aville, AL   ||
  | For          ||
  | Amount       ||
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
  | For          ||
  | Amount       ||
  | Charge       ||
  And with options:
  | Other |

Scenario: a member card is charged, with scanner set, not signed in
  Given members have:
  | uid  | selling   | coFlags |*
  | .ZZC | groceries | tip     |
  And cryptcookie "qid" is "NEWZZB"
  And cookie "scanner" is "NEWZZC"
  And cookie "trust" is "1"
  And cookie "box" is "abcd"
  And these "r_boxes":
  | id | channel | code | boxnum | uid  |*
  | 2  | %TX_WEB | abcd | 12345  | .ZZC |

  When member "?" completes "card/6vm/KDJIAa1" with:
  | op     | amount | desc      |*
  | charge | 10     | groceries |
  Then we say "status": "report tx" with subs:
  | did     | otherName | amount |*
  | charged | Abe One   | $10    |
  And these "txs":
  | xid | created | amount | payer | payee | purpose   | taking |*
  |   1 | %today  |     10 | .ZZA  | .ZZC  | groceries | 1      |
  And we show "You: Our Pub" with:
  | Undo |
  | Scan a CG Card |
  | Tip |
  | Receipt |
  And without:
  | Show My QR |

Scenario: a member card is paid, with scanner set, not signed in
  Given members have:
  | uid  | selling   |*
  | .ZZC | groceries |
  And cryptcookie "qid" is "NEWZZB"
  And cookie "scanner" is "NEWZZC"
  And cookie "trust" is "1"
  And cookie "box" is "abcd"
  And these "r_boxes":
  | id | channel | code | boxnum | uid  |*
  | 2  | %TX_WEB | abcd | 12345  | .ZZC |

  When member "?" completes "card/6vm/KDJIAa1" with:
  | op     | amount | desc      |*
  | pay    | 10     | groceries |
  Then we say "status": "report tx" with subs:
  | did     | otherName | amount |*
  | paid    | Abe One   | $10    |
  And these "txs":
  | xid | created | amount | payer | payee | purpose   | taking |*
  |   1 | %today  |     10 | .ZZC  | .ZZA  | groceries | 0      |

Scenario: a member undoes a charge
  Given members have:
  | uid  | selling   |*
  | .ZZC | groceries |
  And cryptcookie "qid" is "NEWZZB"
  And cookie "scanner" is "NEWZZC"
  And cookie "trust" is "1"
  And cookie "box" is "abcd"
  And these "r_boxes":
  | id | channel | code | boxnum | uid  |*
  | 2  | %TX_WEB | abcd | 12345  | .ZZC |
  And these "txs":
  | xid | created | amount | payer | payee | purpose | taking | boxId |*
  |   3 | %today  |    123 | .ZZA  | .ZZC  | bread   | 1      | 2     |
  
  When member "?" visits "card/undo/xid=3"
  Then these "txs":
  | xid | amt  | uid1 | uid2 | purpose | reversesXid |*
  |   3 |  123 | .ZZA | .ZZC | bread   |             |
  |   4 | -123 | .ZZA | .ZZC | bread   |           3 |
  And we say "status": "report undo|tx desc active" with subs:
  | solution | did      | otherName | amount |*
  | reversed | refunded | Abe One   | $123   |  
  And we message "refunded you" to member ".ZZA" with subs:
  | created | otherName | amount | payerPurpose |*
  | %today  | Our Pub   | $123   | bread        |
  And we message "you refunded" to member ".ZZC" with subs:
  | created | otherName | amount | payeePurpose |*
  | %today  | Abe One   | $123   | bread        |
  And we show "You: Our Pub" with:
  | Scan a CG Card |
  And without:
  | Undo |
  | Show My QR |
  | Tip |
  | Receipt |

Scenario: a member adds a tip
  Given members have:
  | uid  | selling   |*
  | .ZZC | groceries |
  And cryptcookie "qid" is "NEWZZB"
  And cookie "scanner" is "NEWZZC"
  And cookie "trust" is "1"
  And cookie "box" is "abcd"
  And these "r_boxes":
  | id | channel | code | boxnum | uid  |*
  | 2  | %TX_WEB | abcd | 12345  | .ZZC |
  And these "txs":
  | xid | created | amount | payer | payee | purpose   | taking | boxId |*
  |   3 | %today  |     10 | .ZZA  | .ZZC  | groceries | 1      | 2     |

  When member "?" visits "card/tip/xid=3"
  Then we show "" with:
  | No Tip  | |
  | 15% Tip | $1.50 |
  | 20% Tip | $2.00 |
  | 25% Tip | $2.50 |
  | Custom % | |
  | Custom $ | |

  When member "?" visits "card/tip/xid=3&tip=20!"
  Then these "txs":
  | eid | xid | amount | payer | payee | purpose   | type     |*
  |   1 |   3 |     10 | .ZZA  | .ZZC  | groceries | %E_PRIME |
  |   3 |   3 |      2 | .ZZA  | .ZZC  | tip (20%) | %E_AUX   |
  And we show "You: Our Pub" with:
  | Undo |
  | Scan a CG Card |
  | Receipt |
  And without:
  | Show My QR |
  | Tip |
