Feature: Grading
AS a member
I WANT to grade funding proposals
SO as to improve life for everyone

Setup:
  Given members:
  | uid   | fullName | flags  |*
  | .ZZA | Abe One  | ok     |
  | .ZZB | Bea Two  | ok     |
  And these "r_events":
  | id | ctty | type | event         | start        | end          |*
  |  1 | ctty |    P | RFP ($10,000) | %daystart-1w | %daystart-2d |
  |  2 | ctty |    G | Grade         | %daystart    | %daystart+1d |
  And these "r_proposals":
  | id | event | project | overview | categories | purpose | systemic | where | when      | until        | amount | recovery | ctty |*
  | 1  |     1 | Dance   | Fun!     | arts,food  | move    | mystery  | here  | %daystart | %daystart+1d |   1000 |       15 | ctty |
  | 2  |     1 | Play    | Funner   | energy     |  health | very     | there | %daystart | %daystart+2d |   2000 |       48 | ctty |

Scenario: A member grades a proposal
  Given member ".ZZA" has "vote" steps done: "all"
  When member ".ZZA" visits page "community/events"
  Then we show "Community Democracy" with:
  || Common Good Western Mass |
  || Status: Grade from NOW to %mdY+1d |

  When member ".ZZA" completes form "community/events" with values:
  | op      | agree |*
  | gradeIt |    on |
  Then we show "Community Democracy" with:
  | Proposal Grading | (Proposal #1 of 2) |
  | PROPOSAL #1       | Dance |
  || does the project support |
  || does the project promote systemic |
  | Categories  | arts,food |
  | Description | Fun! |
  | When:       | From %mdY |
  | Amount Requested | $1,000 |
  | Recovery    ||
  || Use |
  || Economic Circles |
  || Invitations |
  || Roundups |
  And these "r_ballots":
  | id | question | voter | proxy |*
  |  1 |       -1 |  .ZZA |     0 |
  And these "r_votes":
  | id | ballot | option | grade | gradeMax | displayOrder | text | isVeto |*
  |  1 |      1 |     -1 |    -1 |       -1 |            0 |      |      0 |
  |  2 |      1 |     -2 |    -1 |       -1 |            1 |      |      0 |
  |  3 |      1 |     -3 |    -1 |       -1 |            2 |      |      0 |
  |  4 |      1 |     -4 |    -1 |       -1 |            3 |      |      0 |
  |  5 |      1 |     -5 |    -1 |       -1 |            4 |      |      0 |
  |  6 |      1 |     -6 |    -1 |       -1 |            5 |      |      0 |
  |  7 |      1 |     -7 |    -1 |       -1 |            6 |      |      0 |
  
  When member ".ZZA" completes form "community/events" with values:
  | ballot | question | optionCount | vote0 | vote1 | vote2 | vote3 | vote4 | vote5 | vote6 | option0 | option1 | option2 | option3 | option4 | option5 | option6 | votenote5 | votenote6 | veto6 |*
  |      1 |       -1 |           7 |     1 |     2 |     3 |     4 |     5 |     6 |     7 |       0 |   0.667 |   2.333 | 002.667 |       4 |   4.333 | -0.3333 | good one  | very bad  |    on |
  Then we show "Community Democracy" with:
  | Proposal Grading | (Proposal #2 of 2) |
  | PROPOSAL #2       | Play |
  || does the project support |
  || does the project promote systemic |
  | Categories  | energy |
  | Description | Funner |
  | When:       | From %mdY |
  | Amount Requested | $2,000 |
  | Recovery    ||
  || Percentage |
  || Dividends  |
  And these "r_votes":
  | id | ballot | option |  grade | gradeMax | displayOrder | text     | isVeto |*
  |  1 |      1 |     -1 |      0 |       -1 |            0 |          |      0 |
  |  2 |      1 |     -2 |      2 |       -1 |            1 |          |      0 |
  |  3 |      1 |     -3 |      7 |       -1 |            2 |          |      0 |
  |  4 |      1 |     -4 |      8 |       -1 |            3 |          |      0 |
  |  5 |      1 |     -5 |     12 |       -1 |            4 |          |      0 |
  |  6 |      1 |     -6 |     13 |       -1 |            5 | good one |      0 |
  |  7 |      1 |     -7 |     -2 |       -1 |            6 | very bad |      1 |
  
  When member ".ZZA" visits page "community/events/do=results&eid=2"
  Then we show "Community Democracy" with:
  | project | who     || amount | categories || grade | notes   |
  | Dance   |         || $1,000 | arts,food  || D+    | 2 notes |
  | Play    |         || $2,000 | energy     || E     |         |
  
  When member ".ZZA" visits page "prox/page=ProposalNotes&p=1"
  Then we show "Project Proposal Comments" with:
  | Project | Dance ||
  | eval?   | good one ||
  | recovery? | very bad | VETO! |