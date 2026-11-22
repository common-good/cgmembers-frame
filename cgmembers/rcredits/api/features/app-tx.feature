Feature: App Transactions
AS a member using the CGPay store app
I WANT to identify, charge and pay other members with the app's sign-in token
SO the app talks to one server in one way, and a lost phone's sign-in can be ended
# The store app calls /cgpay/identity, /cgpay/photo and /cgpay/tx through node (server-to-server). The fields and the proof are those of the
# old /api/identity and /api/transactions; the token takes the place of the device id. The old endpoints stay for the old app.

Setup:
  Given members:
  | uid  | fullName | phone  | email | cc  | cc2  | flags  | pass | city | state | balance | activated |*
  | .ZPA | Abe One  | +20001 | a@    | ccA | ccA2 | ok     | Aa1  | Aton | AL    | 1000    | 0         |
  | .ZPB | Bea Two  | +20002 | b@    | ccB |      | ok     |      | Bton | MA    | 1000    | %now0-2w  |
  | .ZPC | Coco Co  | +20003 | c@    | ccC |      | ok,co  |      | Cton | CA    | 1000    | 0         |
  | .ZPE | Eli Five | +20005 | e@    | ccE |      | ok,admin | Ee5 | Eton | NY  | 1000    | 0         |
  And these "u_company":
  | uid  | selling |*
  | .ZPC | stuff   |
  And these "u_relations":
  | main | agent | num | permission |*
  | .ZPC | .ZPA  |   1 | manage     |
  And member ".ZPA" has a token from server "%STAGE" kept as var "tokA"
  And member ".ZPB" has a token from server "%STAGE" kept as var "tokB"
  And member ".ZPE" has a token from server "%STAGE" kept as var "tokE"

# Rule: The token stands in for the device id: the same transaction, with the same proof, goes through.

Scenario: The app charges a customer for a company
  When cgpay posts "tx" with:
  | token | amount | actorId  | otherId | description | created | proof                        | offline | pending |*
  | %tokA | 123    | L6VMDCC0 | K6VMDCB | stuff       | %now    | L6VMDCC0123.00K6VMDCBccB%now | 0       | 0       |
  Then we reply "ok" with JSON:
  | ok   | message                   |*
  | true | You charged Bea Two $123. |
  And these "txs":
  | actorId | uid1 | uid2 | agt1 | agt2 | amt | for2  | created |*
  | .ZPC    | .ZPB | .ZPC | .ZPB | .ZPA | 123 | stuff | %now    |

Scenario: The app pays a customer from the member's own account
  When cgpay posts "tx" with:
  | token | amount | actorId | otherId | description | created | proof                        | offline | pending |*
  | %tokA | -123   | K6VMDCA | K6VMDCB | stuff       | %now    | K6VMDCA-123.00K6VMDCBccB%now | 0       | 0       |
  Then we reply "ok" with JSON:
  | ok   | message                |*
  | true | You paid Bea Two $123. |
  And these "txs":
  | actorId | uid1 | uid2 | amt | for2  | created |*
  | .ZPA    | .ZPA | .ZPB | 123 | stuff | %now    |

Scenario: The app undoes a charge it sent
  Given these "txs":
  | xid | actorId | uid1 | uid2 | agt1 | agt2 | amt | for2  | created | flags |*
  | 1   | .ZPC    | .ZPB | .ZPC | .ZPB | .ZPA | 123 | stuff | %now0   |       |
  When cgpay posts "tx" with:
  | token | amount | actorId  | otherId | description | created | proof                         | offline | pending |*
  | %tokA | -123   | L6VMDCC0 | K6VMDCB | stuff       | %now0   | L6VMDCC0123.00K6VMDCBccB%now0 | 0       | 0       |
  Then we reply "ok" with JSON:
  | ok   | message |*
  | true | deleted |
  And these "txs":
  | xid | actorId | uid1 | uid2 | amt  | reversesXid |*
  | 2   | .ZPC    | .ZPB | .ZPC | -123 | 1           |

Scenario: A transaction with a wrong proof is refused
  When cgpay posts "tx" with:
  | token | amount | actorId  | otherId | description | created | proof                        | offline | pending |*
  | %tokA | 123    | L6VMDCC0 | K6VMDCB | stuff       | %now    | L6VMDCC0123.00K6VMDCBccX%now | 1       | 0       |
  Then we reply "syntax" with: "bad proof"
  And count "txs" is 0

# Rule: The token's member must be the one acting, for an account they may act for.

Scenario: A token acting as another member's agent is refused
  When cgpay posts "tx" with:
  | token | amount | actorId  | otherId | description | created | proof                        | offline | pending |*
  | %tokB | 123    | L6VMDCC0 | K6VMDCB | stuff       | %now    | L6VMDCC0123.00K6VMDCBccB%now | 0       | 0       |
  Then we reply "forbidden" with JSON values:
  | error     |*1
  | bad-actor |
  And count "txs" is 0

Scenario: A token acting as a company with no agent named is refused
  When cgpay posts "tx" with:
  | token | amount | actorId | otherId | description | created | proof                       | offline | pending |*
  | %tokA | 123    | K6VMDCC | K6VMDCB | stuff       | %now    | K6VMDCC123.00K6VMDCBccB%now | 0       | 0       |
  Then we reply "forbidden" with JSON values:
  | error     |*1
  | bad-actor |

Scenario: An admin acts for a company they have no relation with, which names no agent
  When cgpay posts "tx" with:
  | token | amount | actorId | otherId | description | created | proof                       | offline | pending |*
  | %tokE | 123    | K6VMDCC | K6VMDCB | stuff       | %now    | K6VMDCC123.00K6VMDCBccB%now | 0       | 0       |
  Then we reply "ok" with JSON:
  | ok   | message                   |*
  | true | You charged Bea Two $123. |
  And these "txs":
  | actorId | uid1 | uid2 | amt | for2  | created |*
  | .ZPC    | .ZPB | .ZPC | 123 | stuff | %now    |

Scenario: A token acting as another member's own account is refused
  When cgpay posts "tx" with:
  | token | amount | actorId | otherId | description | created | proof                        | offline | pending |*
  | %tokB | -123   | K6VMDCA | K6VMDCB | stuff       | %now    | K6VMDCA-123.00K6VMDCBccB%now | 0       | 0       |
  Then we reply "forbidden" with JSON values:
  | error     |*1
  | bad-actor |

Scenario: A made-up token is refused
  When cgpay posts "tx" with:
  | token        | amount | actorId | otherId | description | created | proof                        | offline | pending |*
  | %STAGE.dummy | -123   | K6VMDCA | K6VMDCB | stuff       | %now    | K6VMDCA-123.00K6VMDCBccB%now | 0       | 0       |
  Then we reply "unauth" with JSON values:
  | error     |*1
  | bad-token |

# Rule: The app identifies the account it scanned, and shows its photo when the card's code was scanned.

Scenario: The app identifies a customer
  When cgpay posts "identity" with:
  | token | actorId  | otherId    |*
  | %tokA | L6VMDCC0 | K6VMDCBccB |
  Then we reply "got" with JSON:
  | name    | agent | location | limit | creditLine | avgBalance | trustRatio | since    | selling | isell   |*
  | Bea Two |       | Bton, MA |  1000 |          0 |          0 |          0 | %now0-2w | []      | [stuff] |

Scenario: The app identifies a customer for another member's account
  When cgpay posts "identity" with:
  | token | actorId  | otherId    |*
  | %tokB | L6VMDCC0 | K6VMDCBccB |
  Then we reply "forbidden" with JSON values:
  | error     |*1
  | bad-actor |

Scenario: The app shows a customer's photo
  Given member ".ZPB" has photo "pictureB"
  When cgpay posts "photo" with:
  | token | actorId  | otherId    |*
  | %tokA | L6VMDCC0 | K6VMDCBccB |
  Then we reply "ok" with JSON values:
  | photo                               |*1
  | data:image/jpeg;base64,cGljdHVyZUI= |

Scenario: The app asks for a photo without the card's code
  When cgpay posts "photo" with:
  | token | actorId  | otherId |*
  | %tokA | L6VMDCC0 | K6VMDCB |
  Then we reply "notfound" with JSON values:
  | error     |*1
  | not-found |
