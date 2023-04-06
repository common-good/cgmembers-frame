Feature: Get Transactions
AS a member
I WANT to see my recent transactions
SO I can confirm a recent purchase or charge

Setup:
  Given members:
  | uid  | fullName | phone  | email | cc  | cc2  | flags      | pass | city | state | balance | jid  | crumbs | created |*
  | .ZPA | Abe One  | +20001 | a@    | ccA | ccA2 | ok,roundup | Aa1  | Aton | AL    | 1000    |    0 |      0 | %now-1m |
  | .ZPB | Bea Two  | +20002 | b@    | ccB |      | ok         |      | Bton | MA    | 2000    | .ZPF |      0 | %now-2m |
  | .ZPC | Coco Co  | +20003 | c@    | ccC |      | ok,co      |      | Cton | CA    | 3000    |    0 |    0.7 | %now-3m |
  | .ZPF | For Co   | +20006 | f@    | ccF |      | co         |      | Fton | FL    | 4000    | .ZPB |    1.5 | %now-4m |
  And these "r_boxes":
  | uid  | code |*
  | .ZPC | devC |
  And these "u_relations":
  | main | agent | num | permission |*
  | .ZPC | .ZPA  |   1 | buy        |
  | .ZPC | .ZPB  |   2 | scan       |
  | .ZPF | .ZPA  |   1 | manage     |
  And these "tx_rules":
  | id | action | from | to   | payer | payeeType | amount | portion |*
  |  1 | surtx  | .ZPA | .ZPC | .ZPA  | anyCo     |      0 |    0.03 |
  |  2 | surtx  | .ZPA | .ZPC | .ZPA  | anyCo     |      4 |    1.23 |
  And these "txs":
  | xid | created | amount | uid1 | uid2 | agt2 | purpose |*
  |   1 | %now-6d |     12 | .ZPA | .ZPC | .ZPB | alpha   |
  |   2 | %now-5d |     23 | .ZPB | .ZPC | .ZPC | beta    |
  |   3 | %now-4d |     34 | .ZPC | .ZPF | .ZPF | gamma   |
  |   4 | %now-3d |     56 | .ZPA | .ZPB | .ZPB | delta   |
  |   5 | %now-2d |     78 | .ZPF | .ZPA | .ZPA | epsil   |
  |   6 | %now-1d |     90 | .ZPA | .ZPC | .ZPC | theta   |
  Then balances:
  | uid  | balance |*
  | .ZPA |     920 |
  | .ZPC |    3091 |

# GET /transactions (version, deviceId, actorId, lastTx)
#   -> {balance, surtxs, crumbs, roundups, txs: [{xid, amount, other: {}, description, created}, …]}
#   where other: {name, avatar}

Scenario: The app asks for recent transactions
  Given var "surtxs" is JSON:
  | amount | portion |*
  |      4 |    1.26 |
  And var "zpb" is JSON:
  | name    | location |*
  | Bea Two | Bton, MA |
  And var "zpc" is JSON:
  | name    | location |*
  | Coco Co | Cton, MA |
  And var "cb" is JSON:
  | name    | location |*
  | Coco Co | Cton, CA |
  And var "zpf" is JSON:
  | name    | location |*
  | For Co  | Fton, FL |
  And var "txs" is JSON:
  | xid | amount | other | description | created |*
  |   6 |    -90 | %cb   | theta       | %ymd-1d |
  |   5 |     78 | %zpf  | epsil       | %ymd-2d |
  |   4 |    -56 | %zpb  | delta       | %ymd-3d |
  |   1 |    -12 | %zpc  | alpha       | %ymd-6d |
  When app gets "transactions" with:
  | version | deviceId | actorId | lastTx  |*
  | 400     | devA     | K6VMDCA | %now-7d |
  Then we reply "got" with JSON:
  | balance | surtxs  | crumbs | roundups | txs  |*
  | 920     | %surtxs |      0 |     true | %txs |
Skip
Scenario: The app asks to charge a customer with a missing parameter
  When app posts "transactions" with:
  | version | deviceId | amount | actorId | otherId | description | created | proof                       | offline |*
  | 400     | devC     | 123    | K6VMDCC | K6VMDCB |             | %now    | K6VMDCC123.00K6VMDCBccB%now | false   |
  Then we reply "syntax" with: "?"

Scenario: The app asks to charge a customer with a bad actorId
  When app posts "transactions" with:
  | version | deviceId | amount | actorId | otherId | description | created | proof                       | offline |*
  | 400     | devC     | 123    | K6VMDCX | K6VMDCB | stuff       | %now    | K6VMDCC123.00K6VMDCBccB%now | false   |
  Then we reply "unauth"

Scenario: The app asks to charge a customer with a bad otherId
  When app posts "transactions" with:
  | version | deviceId | amount | actorId | otherId    | description | created | proof                       | offline |*
  | 400     | devC     | 123    | K6VMDCC | K6VMDCXccx | stuff       | %now    | K6VMDCC123.00K6VMDCBccB%now | false   |
  Then we reply "ok" with JSON:
  | ok    | message                                 |*
  | false | That is not an active %PROJECT account. |

Scenario: The app asks to charge a customer with a bad otherId offline
  When app posts "transactions" with:
  | version | deviceId | amount | actorId | otherId    | description | created | proof                       | offline |*
  | 400     | devC     | 123    | K6VMDCC | K6VMDCXccx | stuff       | %now    | K6VMDCC123.00K6VMDCBccB%now | true    |
  Then we reply "ok"
  And we tell Admin "bad card code" with subs:
  | acctId     | code |*
  | K6VMDCXccx |      |
  And these "tx_bads":
  | version | deviceId | amount | actorId | otherId    | description | created | proof                       | offline |*
  | 400     | devC     | 123    | K6VMDCC | K6VMDCXccx | stuff       | %now    | K6VMDCC123.00K6VMDCBccB%now | 1       |
  
Scenario: The app asks to charge a customer with a bad amount
  When app posts "transactions" with:
  | version | deviceId | amount | actorId | otherId | description | created | proof                       | offline |*
  | 400     | devC     | 1.2.3  | K6VMDCC | K6VMDCB | stuff       | %now    | K6VMDCC123.00K6VMDCBccB%now | false   |
  Then we reply "syntax" with: "?"

Scenario: The app asks to charge a customer with an amount out of range
  When app posts "transactions" with:
  | version | deviceId | amount             | actorId | otherId | description | created | proof                       | offline |*
  | 400     | devC     | %(%MAX_AMOUNT + 1) | K6VMDCC | K6VMDCB | stuff       | %now    | K6VMDCC123.00K6VMDCBccB%now | false   |
  Then we reply "syntax" with: "?"

Scenario: The app asks to charge a customer with a bad date
  When app posts "transactions" with:
  | version | deviceId | amount | actorId | otherId | description | created | proof                       | offline |*
  | 400     | devC     | 123    | K6VMDCC | K6VMDCB | stuff       | abc     | K6VMDCC123.00K6VMDCBccB%now | false   |
  Then we reply "syntax" with: "?"

Scenario: The app asks to charge a customer with a date out of range
  When app posts "transactions" with:
  | version | deviceId | amount | actorId | otherId | description | created | proof                       | offline |*
  | 400     | devC     | 123    | K6VMDCC | K6VMDCB | stuff       | 123     | K6VMDCC123.00K6VMDCBccB%now | false   |
  Then we reply "syntax" with: "?"

Scenario: The app asks to charge a customer with a bad proof
  When app posts "transactions" with:
  | version | deviceId | amount | actorId | otherId | description | created | proof | offline |*
  | 400     | devC     | 123    | K6VMDCC | K6VMDCB | stuff       | %now    | ccX   | false   |
  Then we reply "syntax" with: "?"

Scenario: The app asks to charge a customer who has insufficient funds
  When app posts "transactions" with:
  | version | deviceId | amount | actorId | otherId | description | created | proof                         | offline |*
  | 400     | devC     | 12300  | K6VMDCC | K6VMDCB | stuff       | %now    | K6VMDCC12300.00K6VMDCBccB%now | false   |
  Then we reply "ok" with JSON:
  | ok    | message                                                                                    |*
  | false | Bea Two is $11,300 short for this request (do NOT try again). Available balance is $1,000. |
