Feature: App Cash Register
AS a manager of a company
I WANT to leave the store app on the company's account, as a cash register
SO a cashier can charge customers but reach nothing else: not the company's dashboard, its money, or my own accounts
# The app locks its token to the company (/cgpay/lock); the server then gives that token the company alone, until a manager
# unlocks it for a while (/cgpay/unlock, with the token owner's password or a manager's card).

Setup:
  Given members:
  | uid  | fullName | phone  | email | cc  | cc2  | flags  | pass | city | state |*
  | .ZPA | Abe One  | +20001 | a@    | ccA | ccA2 | ok     | Aa1  | Aton | AL    |
  | .ZPB | Bea Two  | +20002 | b@    | ccB | ccB2 | ok     | Bb2  | Bton | MA    |
  | .ZPC | Coco Co  | +20003 | c@    | ccC |      | ok,co  |      | Cton | CA    |
  | .ZPF | Flo Six  | +20006 | f@    | ccF | ccF2 | ok     | Ff6  | Fton | FL    |
  And these "u_relations":
  | main | agent | num | permission |*
  | .ZPC | .ZPA  |   1 | manage     |
  | .ZPC | .ZPB  |   2 | manage     |
  | .ZPC | .ZPF  |   3 | buy        |
  And member ".ZPA" has "card" steps done: "all"
  And member ".ZPB" has "card" steps done: "all"
  And member ".ZPF" has "card" steps done: "all"
  And cgpay posts "signin" with:
  | identifier | password | label |*
  | .ZPA       | Aa1      | Pixel |
  And reply "token" is kept as var "token"

# Rule: The app acts for a company only for its managers.

Scenario: A manager gets the company with their own account
  When cgpay posts "device" with:
  | token  |*
  | %token |
  Then reply list "accounts" is:
  | uid  | name    |*
  | .ZPA | Abe One |
  | .ZPC | Coco Co |

Scenario: Someone who may buy for a company but not manage it does not get the company
  Given cgpay posts "signin" with:
  | identifier | password | label |*
  | .ZPF       | Ff6      | Pixel |
  And reply "token" is kept as var "token"
  When cgpay posts "device" with:
  | token  |*
  | %token |
  Then reply list "accounts" is:
  | uid  |*
  | .ZPF |

Scenario: Someone who does not manage the company cannot make the app its cash register
  Given cgpay posts "signin" with:
  | identifier | password | label |*
  | .ZPF       | Ff6      | Pixel |
  And reply "token" is kept as var "token"
  When cgpay posts "lock" with:
  | token  | account |*
  | %token | .ZPC    |
  Then we reply "forbidden" with JSON values:
  | error       |*1
  | not-allowed |

Scenario: A personal account cannot be a cash register
  When cgpay posts "lock" with:
  | token  | account |*
  | %token | .ZPA    |
  Then we reply "forbidden" with JSON values:
  | error       |*1
  | not-allowed |

# Rule: A locked token works for its company alone.

Scenario: A manager makes the app the company's cash register
  When cgpay posts "lock" with:
  | token  | account |*
  | %token | .ZPC    |
  Then we reply "ok" with JSON values:
  | uid  | lockedTo | admin |*1
  | .ZPA | .ZPC   | 0     |
  And reply list "accounts" is:
  | uid  |*
  | .ZPC |
  And these "app_tokens":
  | uid  | lockedTo | unlockedUntil |*
  | .ZPA | .ZPC     |               |

Scenario: A locked token gets only the company's credentials
  Given cgpay posts "lock" with:
  | token  | account |*
  | %token | .ZPC    |
  When cgpay posts "device" with:
  | token  |*
  | %token |
  Then we reply "ok" with JSON values:
  | uid  | lockedTo |*1
  | .ZPA | .ZPC   |
  And reply list "accounts" is:
  | uid  |*
  | .ZPC |

Scenario: A locked token opens no live page
  Given cgpay posts "lock" with:
  | token  | account |*
  | %token | .ZPC    |
  When cgpay posts "enter" with:
  | account | token  |*
  | .ZPC    | %token |
  Then we reply "forbidden" with JSON values:
  | error  |*1
  | locked |
  And count "app_codes" is 0

Scenario: A locked token cannot fetch another account
  Given cgpay posts "lock" with:
  | token  | account |*
  | %token | .ZPC    |
  When cgpay posts "account" with:
  | token  | who  |*
  | %token | .ZPA |
  Then we reply "forbidden" with JSON values:
  | error  |*1
  | locked |

Scenario: A locked token cannot unlock itself
  Given cgpay posts "lock" with:
  | token  | account |*
  | %token | .ZPC    |
  When cgpay posts "lock" with:
  | token  | account |*
  | %token | 0       |
  Then we reply "forbidden" with JSON values:
  | error  |*1
  | locked |
  And these "app_tokens":
  | uid  | lockedTo |*
  | .ZPA | .ZPC     |

# Rule: A manager's card, or the token owner's password, unlocks the token until it goes unused for two minutes.

Scenario: Another manager's card unlocks the cash register
  Given cgpay posts "lock" with:
  | token  | account |*
  | %token | .ZPC    |
  And var "card" is the "real" card of ".ZPB"
  When cgpay posts "unlock" with:
  | token  | qr    |*
  | %token | %card |
  Then we reply "ok" with JSON values:
  | ok | unlockedBy | uid  | lockedTo |*1
  | 1  | .ZPB | .ZPA | 0      |
  And reply list "accounts" is:
  | uid  |*
  | .ZPA |
  | .ZPC |
  And these "app_tokens":
  | uid  | lockedTo | unlockedUntil |*
  | .ZPA | .ZPC     | %now+2n       |

Scenario: A company card of a manager unlocks the cash register
  Given cgpay posts "lock" with:
  | token  | account |*
  | %token | .ZPC    |
  And var "card" is the "real" card of ".ZPC:.ZPB"
  When cgpay posts "unlock" with:
  | token  | qr    |*
  | %token | %card |
  Then we reply "ok" with JSON values:
  | ok | unlockedBy | lockedTo |*1
  | 1  | .ZPB | 0      |

Scenario: The card of someone who does not manage the company does not unlock
  Given cgpay posts "lock" with:
  | token  | account |*
  | %token | .ZPC    |
  And var "card" is the "real" card of ".ZPF"
  When cgpay posts "unlock" with:
  | token  | qr    |*
  | %token | %card |
  Then we reply "forbidden" with JSON values:
  | error       |*1
  | not-manager |

Scenario: A test card does not unlock a real cash register
  Given cgpay posts "lock" with:
  | token  | account |*
  | %token | .ZPC    |
  And var "card" is the "test" card of ".ZPB"
  When cgpay posts "unlock" with:
  | token  | qr    |*
  | %token | %card |
  Then we reply "denied" with JSON values:
  | error      |*1
  | wrong-kind |

Scenario: A card does not unlock a token that is not locked
  Given var "card" is the "real" card of ".ZPB"
  When cgpay posts "unlock" with:
  | token  | qr    |*
  | %token | %card |
  Then we reply "forbidden" with JSON values:
  | error      |*1
  | not-locked |

Scenario: The token owner's password unlocks the cash register
  Given cgpay posts "lock" with:
  | token  | account |*
  | %token | .ZPC    |
  When cgpay posts "unlock" with:
  | token  | password |*
  | %token | Aa1      |
  Then we reply "ok" with JSON values:
  | ok | unlockedBy | lockedTo |*1
  | 1  | .ZPA | 0      |

Scenario: An unlock ends after two minutes without use
  Given cgpay posts "lock" with:
  | token  | account |*
  | %token | .ZPC    |
  And cgpay posts "unlock" with:
  | token  | password |*
  | %token | Aa1      |
  And the time now is "%now+3n"
  When cgpay posts "device" with:
  | token  |*
  | %token |
  Then we reply "ok" with JSON values:
  | lockedTo |*1
  | .ZPC   |

Scenario: An unlock in use lasts
  Given cgpay posts "lock" with:
  | token  | account |*
  | %token | .ZPC    |
  And cgpay posts "unlock" with:
  | token  | password |*
  | %token | Aa1      |
  And the time now is "%now+1n"
  And cgpay posts "device" with:
  | token  |*
  | %token |
  And the time now is "%now+3n"
  When cgpay posts "device" with:
  | token  |*
  | %token |
  Then we reply "ok" with JSON values:
  | lockedTo |*1
  | 0      |

Scenario: Locking again ends the unlock
  Given cgpay posts "lock" with:
  | token  | account |*
  | %token | .ZPC    |
  And cgpay posts "unlock" with:
  | token  | password |*
  | %token | Aa1      |
  When cgpay posts "lock" with:
  | token  |*
  | %token |
  Then we reply "ok" with JSON values:
  | lockedTo |*1
  | .ZPC   |

Scenario: An unlocked cash register can be let go
  Given cgpay posts "lock" with:
  | token  | account |*
  | %token | .ZPC    |
  And cgpay posts "unlock" with:
  | token  | password |*
  | %token | Aa1      |
  When cgpay posts "lock" with:
  | token  | account |*
  | %token | 0       |
  Then we reply "ok" with JSON values:
  | lockedTo |*1
  | 0      |
  And these "app_tokens":
  | uid  | lockedTo |*
  | .ZPA |          |
