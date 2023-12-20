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
  | .ZPC | .ZPB  |   2 | sell       |
  | .ZPF | .ZPA  |   1 | manage     |
  And these "tx_rules":
  | id | action | from | to   | payer | payeeType | amount | portion |*
  |  1 | surtx  | .ZPA | cgf  | .ZPA  | anyCo     |      0 |    0.25 |
  |  2 | surtx  | .ZPA | .ZPF | .ZPA  | anyCo     |      4 |       0 |
  And these "txs":
  | eid | xid | created | amount | uid1 | uid2 | agt2 | purpose |*
  |   1 |   1 | %now-6d |     12 | .ZPA | .ZPC | .ZPB | alpha   |
  |   2 |   2 | %now-5d |     23 | .ZPB | .ZPC | .ZPC | beta    |
  |   3 |   3 | %now-4d |     34 | .ZPC | .ZPF | .ZPF | gamma   |
  |   4 |   4 | %now-3d |     56 | .ZPA | .ZPB | .ZPB | delta   |
  |   5 |   5 | %now-2d |     78 | .ZPF | .ZPA | .ZPA | epsil   |
  |   6 |   6 | %now-1d |    900 | .ZPA | .ZPC | .ZPC | theta   |
  And these "txs":
  | eid | xid | created | amount | uid1 | uid2 | agt2 | purpose | type   |*
  |   7 |   6 | %now-1d |      4 | .ZPA | .ZPF | .ZPF | gift    | %E_AUX |
  |   8 |   6 | %now-1d |    225 | .ZPA | cgf  | cgf  | gift    | %E_AUX |
  And these "tx_requests":
  | nvid | created | amount | payer | payee | purpose |*
  |    1 | %now-7d |     45 |  .ZPB | .ZPA  | lunch   |
  Then balances:
  | uid  | balance |*
  | .ZPA |    -119 |
  | .ZPC |    3901 |

# GET /info (version, deviceId, actorId, lastTx)
#   -> {balance, surtxs, txs: [{xid, amount, accountId, name, description, created}, …]}

Scenario: The app asks for recent transactions
  Given var "surtxs" is JSON:
  | amount | portion | crumbs | roundup |*1
  |      4 |    0.25 |      0 |    true |
  And var "txs" is JSON:
  | pending | extra   | xid | amount | accountId | name    | description | created |*
  |       1 |       0 |   0 |     45 | K6VMDCB   | Bea Two | lunch       | %now-7d |
  |         | -229.00 |   6 |   -900 | K6VMDCC   | Coco Co | theta       | %now-1d |
  |         |       0 |   5 |     78 | K6VMDCF   | For Co  | epsil       | %now-2d |
  |         |       0 |   4 |    -56 | K6VMDCB   | Bea Two | delta       | %now-3d |
#  |         |       0 |   1 |    -12 | L6VMDCC1  | Coco Co | alpha       | %now-6d |
  When app posts "info" with:
  | version | deviceId | actorId | count  |*
  | 400     | devA     | K6VMDCA | 3      |
  Then we reply "ok" with JSON values:
  | balance | surtxs  | txs  |*1
  | -119    | %surtxs | %txs |

Scenario: The app asks for recent transactions with a missing parameter
  When app posts "info" with:
  | version | deviceId | actorId |*
  | 400     | devC     | K6VMDCC |
  Then we reply "syntax" with: "?"

Scenario: The app asks for recent transactions with a bad actorId
  When app posts "info" with:
  | version | deviceId | actorId | count |*
  | 400     | devC     | K6VMDCX | 3     |
  Then we reply "unauth"

Scenario: The app asks for recent transactions with a bad count
  When app posts "info" with:
  | version | deviceId | actorId | count    |*
  | 400     | devC     | K6VMDCC | whatever |
  Then we reply "syntax" with: "?"

Scenario: The app asks for recent transactions with a count out of range
  When app posts "info" with:
  | version | deviceId | actorId | count  |*
  | 400     | devC     | K6VMDCC | -1     |
  Then we reply "syntax" with: "?"

