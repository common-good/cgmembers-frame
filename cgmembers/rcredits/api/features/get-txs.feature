Feature: Get Transactions
AS a member
I WANT to see my recent transactions
SO I can confirm a recent purchase or charge

Setup:
  Given members:
  | uid  | fullName | phone  | email | cc  | cc2  | flags      | pass | city | state | balance | jid  | crumbs | created |*
  | .ZZA | Abe One  | +20001 | a@    | ccA | ccA2 | ok,roundup | Aa1  | Aton | AL    | 1000    |    0 |      0 | %now-1m |
  | .ZZB | Bea Two  | +20002 | b@    | ccB |      | ok         |      | Bton | MA    | 2000    | .ZZF |      0 | %now-2m |
  | .ZZC | Coco Co  | +20003 | c@    | ccC |      | ok,co      |      | Cton | CA    | 3000    |    0 |    0.7 | %now-3m |
  | .ZZF | For Co   | +20006 | f@    | ccF |      | co         |      | Fton | FL    | 4000    | .ZZB |    1.5 | %now-4m |
  And these "r_boxes":
  | uid  | code |*
  | .ZZC | devC |
  And these "u_relations":
  | main | agent | num | permission |*
  | .ZZC | .ZZA  |   1 | buy        |
  | .ZZC | .ZZB  |   2 | scan       |
  | .ZZF | .ZZA  |   1 | manage     |
  And these "tx_rules":
  | id | action | from | to   | payer | payeeType | amount | portion |*
  |  1 | surtx  | .ZZA | .ZZC | .ZZA  | anyCo     |      0 |    0.03 |
  |  2 | surtx  | .ZZA | .ZZC | .ZZA  | anyCo     |      4 |    1.23 |
  And these "txs":
  | xid | created | amount | uid1 | uid2 | agt2 | purpose |*
  |   1 | %now-6d |     12 | .ZZA | .ZZC | .ZZB | alpha   |
  |   2 | %now-5d |     23 | .ZZB | .ZZC | .ZZC | beta    |
  |   3 | %now-4d |     34 | .ZZC | .ZZF | .ZZF | gamma   |
  |   4 | %now-3d |     56 | .ZZA | .ZZB | .ZZB | delta   |
  |   5 | %now-2d |     78 | .ZZF | .ZZA | .ZZA | epsil   |
  |   6 | %now-1d |     90 | .ZZA | .ZZC | .ZZC | theta   |
  Then balances:
  | uid  | balance |*
  | .ZZA |     920 |

# GET /transactions (version, deviceId, actorId, lastTx)
#   -> {balance, surtxs, crumbs, roundups, txs: [{xid, amount, other: {}, description, created}, …]}
#   where other: {name, avatar}

Scenario: The app asks for recent transactions
  Given var "surtxs" is JSON:
  | amount | portion |*
  |      4 |    1.23 |
  And var "zzb" is JSON:
  | name    | location | avatar |*
  | Bea Two | Bton, MA | ?      |
  And var "zzc" is JSON:
  | name    | location | avatar |*
  | Coco Co | Bton, MA | ?      |
  And var "zzcb" is JSON:
  | name    | location | avatar |*
  | Coco Co | Cton, CA | ?      |
  And var "zzf" is JSON:
  | name    | location | avatar |*
  | For Co  | Fton, FL | ?      |
  And var "txs" is JSON:
  | xid | amount | other | description | created |*
  |   6 |     90 | %zzcb | theta       | %ymd-1d |
  |   5 |     78 | %zzf  | epsil       | %ymd-2d |
  |   4 |     56 | %zzb  | delta       | %ymd-3d |
  |   1 |     12 | %zzc  | alpha       | %ymd-6d |
  When app gets "transactions" with:
  | version | deviceId | actorId | lastTx  |*
  | 400     | devC     | K6VMDJK | %now-7d |
  Then we reply "got" with JSON:
  | balance | surtxs  | crumbs | roundups | txs  |*
  |     920 | %surtxs |      0 |     true | %txs |
Skip
Scenario: The app asks to charge a customer with a missing parameter
  When app posts "transactions" with:
  | version | deviceId | amount | actorId | otherId | description | created | proof                       | offline |*
  | 400     | devC     | 123    | K6VMDJK | K6VMDJJ |             | %now    | K6VMDJK123.00K6VMDJJccB%now | false   |
  Then we reply "syntax" with: "?"

Scenario: The app asks to charge a customer with a bad actorId
  When app posts "transactions" with:
  | version | deviceId | amount | actorId | otherId | description | created | proof                       | offline |*
  | 400     | devC     | 123    | K6VMDJX | K6VMDJJ | stuff       | %now    | K6VMDJK123.00K6VMDJJccB%now | false   |
  Then we reply "unauth"
  
Scenario: The app asks to charge a customer with a bad otherId
  When app posts "transactions" with:
  | version | deviceId | amount | actorId | otherId    | description | created | proof                       | offline |*
  | 400     | devC     | 123    | K6VMDJK | K6VMDJXccx | stuff       | %now    | K6VMDJK123.00K6VMDJJccB%now | false   |
  Then we reply "ok" with JSON:
  | ok    | message                                 |*
  | false | That is not an active %PROJECT account. |

Scenario: The app asks to charge a customer with a bad otherId offline
  When app posts "transactions" with:
  | version | deviceId | amount | actorId | otherId    | description | created | proof                       | offline |*
  | 400     | devC     | 123    | K6VMDJK | K6VMDJXccx | stuff       | %now    | K6VMDJK123.00K6VMDJJccB%now | true    |
  Then we reply "ok"
  And we tell Admin "bad card code" with subs:
  | acctId     | code |*
  | K6VMDJXccx |      |
  And these "tx_bads":
  | version | deviceId | amount | actorId | otherId    | description | created | proof                       | offline |*
  | 400     | devC     | 123    | K6VMDJK | K6VMDJXccx | stuff       | %now    | K6VMDJK123.00K6VMDJJccB%now | 1       |
  
Scenario: The app asks to charge a customer with a bad amount
  When app posts "transactions" with:
  | version | deviceId | amount | actorId | otherId | description | created | proof                       | offline |*
  | 400     | devC     | 1.2.3  | K6VMDJK | K6VMDJJ | stuff       | %now    | K6VMDJK123.00K6VMDJJccB%now | false   |
  Then we reply "syntax" with: "?"

Scenario: The app asks to charge a customer with an amount out of range
  When app posts "transactions" with:
  | version | deviceId | amount             | actorId | otherId | description | created | proof                       | offline |*
  | 400     | devC     | %(%MAX_AMOUNT + 1) | K6VMDJK | K6VMDJJ | stuff       | %now    | K6VMDJK123.00K6VMDJJccB%now | false   |
  Then we reply "syntax" with: "?"

Scenario: The app asks to charge a customer with a bad date
  When app posts "transactions" with:
  | version | deviceId | amount | actorId | otherId | description | created | proof                       | offline |*
  | 400     | devC     | 123    | K6VMDJK | K6VMDJJ | stuff       | abc     | K6VMDJK123.00K6VMDJJccB%now | false   |
  Then we reply "syntax" with: "?"

Scenario: The app asks to charge a customer with a date out of range
  When app posts "transactions" with:
  | version | deviceId | amount | actorId | otherId | description | created | proof                       | offline |*
  | 400     | devC     | 123    | K6VMDJK | K6VMDJJ | stuff       | 123     | K6VMDJK123.00K6VMDJJccB%now | false   |
  Then we reply "syntax" with: "?"

Scenario: The app asks to charge a customer with a bad proof
  When app posts "transactions" with:
  | version | deviceId | amount | actorId | otherId | description | created | proof | offline |*
  | 400     | devC     | 123    | K6VMDJK | K6VMDJJ | stuff       | %now    | ccX   | false   |
  Then we reply "syntax" with: "?"

Scenario: The app asks to charge a customer who has insufficient funds
  When app posts "transactions" with:
  | version | deviceId | amount | actorId | otherId | description | created | proof                         | offline |*
  | 400     | devC     | 12300  | K6VMDJK | K6VMDJJ | stuff       | %now    | K6VMDJK12300.00K6VMDJJccB%now | false   |
  Then we reply "ok" with JSON:
  | ok    | message                                                                                    |*
  | false | Bea Two is $11,300 short for this request (do NOT try again). Available balance is $1,000. |
