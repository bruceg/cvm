-------------------------------------------------------------------------------
Changes in version 0.12

- Fixed bug: the MySQL code never freed its results, leading to a rather
  significant memory leak.

- Handle lost connections by reconnecting and retrying the query (once).

Development of this version has been sponsored by FutureQuest, Inc.
ossi@FutureQuest.net  http://www.FutureQuest.net/
-------------------------------------------------------------------------------
Changes in version 0.11

- Added a mailbox path column to the required query results.

- There is no longer a default query, as it won't likely work for
  anybody.

Development of this version has been sponsored by FutureQuest, Inc.
ossi@FutureQuest.net  http://www.FutureQuest.net/
-------------------------------------------------------------------------------
