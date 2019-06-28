
--
-- Indexes for dumped tables
--

--
-- Indexes for table `block`
--
ALTER TABLE `block`
  ADD PRIMARY KEY (`bid`),
  ADD UNIQUE KEY `tmd` (`theme`,`module`,`delta`),
  ADD KEY `list` (`theme`,`status`,`region`,`weight`,`module`);

--
-- Indexes for table `cache`
--
ALTER TABLE `cache`
  ADD PRIMARY KEY (`cid`),
  ADD KEY `expire` (`expire`);

--
-- Indexes for table `cache_bootstrap`
--
ALTER TABLE `cache_bootstrap`
  ADD PRIMARY KEY (`cid`),
  ADD KEY `expire` (`expire`);

--
-- Indexes for table `cache_form`
--
ALTER TABLE `cache_form`
  ADD PRIMARY KEY (`cid`),
  ADD KEY `expire` (`expire`);

--
-- Indexes for table `cache_menu`
--
ALTER TABLE `cache_menu`
  ADD PRIMARY KEY (`cid`),
  ADD KEY `expire` (`expire`);

--
-- Indexes for table `flood`
--
ALTER TABLE `flood`
  ADD PRIMARY KEY (`fid`),
  ADD KEY `allow` (`event`,`identifier`,`timestamp`),
  ADD KEY `purge` (`expiration`);

--
-- Indexes for table `legacy_r_txs`
--
ALTER TABLE `legacy_r_txs`
  ADD PRIMARY KEY (`xid`),
  ADD KEY `payer` (`payer`),
  ADD KEY `payee` (`payee`),
  ADD KEY `created` (`created`);

--
-- Indexes for table `menu_links`
--
ALTER TABLE `menu_links`
  ADD PRIMARY KEY (`mlid`),
  ADD KEY `path_menu` (`link_path`(128),`menu_name`),
  ADD KEY `menu_plid_expand_child` (`menu_name`,`plid`,`expanded`,`has_children`),
  ADD KEY `menu_parents` (`menu_name`,`p1`,`p2`,`p3`,`p4`,`p5`,`p6`,`p7`,`p8`,`p9`),
  ADD KEY `router_path` (`router_path`(128));

--
-- Indexes for table `menu_router`
--
ALTER TABLE `menu_router`
  ADD PRIMARY KEY (`path`),
  ADD KEY `fit` (`fit`),
  ADD KEY `tab_parent` (`tab_parent`(64),`weight`,`title`),
  ADD KEY `tab_root_weight_title` (`tab_root`(64),`weight`,`title`);

--
-- Indexes for table `phinxlog`
--
ALTER TABLE `phinxlog`
  ADD PRIMARY KEY (`version`);

--
-- Indexes for table `queue`
--
ALTER TABLE `queue`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `registry`
--
ALTER TABLE `registry`
  ADD PRIMARY KEY (`name`,`type`),
  ADD KEY `hook` (`type`,`weight`,`module`);

--
-- Indexes for table `registry_file`
--
ALTER TABLE `registry_file`
  ADD PRIMARY KEY (`filename`);

--
-- Indexes for table `r_areas`
--
ALTER TABLE `r_areas`
  ADD PRIMARY KEY (`area_code`);

--
-- Indexes for table `r_bad`
--
ALTER TABLE `r_bad`
  ADD PRIMARY KEY (`created`);

--
-- Indexes for table `r_ballots`
--
ALTER TABLE `r_ballots`
  ADD PRIMARY KEY (`id`),
  ADD KEY `question` (`question`),
  ADD KEY `voter` (`voter`),
  ADD KEY `proxy` (`proxy`);

--
-- Indexes for table `r_banks`
--
ALTER TABLE `r_banks`
  ADD PRIMARY KEY (`route`),
  ADD KEY `newroute` (`newRoute`);

--
-- Indexes for table `r_boxes`
--
ALTER TABLE `r_boxes`
  ADD PRIMARY KEY (`id`),
  ADD KEY `uid` (`uid`);

--
-- Indexes for table `r_changes`
--
ALTER TABLE `r_changes`
  ADD PRIMARY KEY (`id`),
  ADD KEY `uid` (`uid`);

--
-- Indexes for table `r_company`
--
ALTER TABLE `r_company`
  ADD PRIMARY KEY (`uid`);

--
-- Indexes for table `r_countries`
--
ALTER TABLE `r_countries`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `name_iso_code` (`name`,`iso_code`),
  ADD KEY `address_format_id` (`address_format_id`),
  ADD KEY `region_id` (`region_id`);

--
-- Indexes for table `r_coupated`
--
ALTER TABLE `r_coupated`
  ADD PRIMARY KEY (`id`),
  ADD KEY `uid` (`uid`),
  ADD KEY `coupid` (`coupid`);

--
-- Indexes for table `r_coupons`
--
ALTER TABLE `r_coupons`
  ADD PRIMARY KEY (`coupid`),
  ADD KEY `fromId` (`fromId`);

--
-- Indexes for table `r_criteria`
--
ALTER TABLE `r_criteria`
  ADD PRIMARY KEY (`id`),
  ADD KEY `ctty` (`ctty`);

--
-- Indexes for table `r_do`
--
ALTER TABLE `r_do`
  ADD PRIMARY KEY (`doid`),
  ADD KEY `uid` (`uid`);

--
-- Indexes for table `r_events`
--
ALTER TABLE `r_events`
  ADD PRIMARY KEY (`id`),
  ADD KEY `ctty` (`ctty`);

--
-- Indexes for table `r_gifts`
--
ALTER TABLE `r_gifts`
  ADD PRIMARY KEY (`donid`),
  ADD KEY `uid` (`uid`);

--
-- Indexes for table `r_honors`
--
ALTER TABLE `r_honors`
  ADD PRIMARY KEY (`id`),
  ADD KEY `uid` (`uid`);

--
-- Indexes for table `r_industries`
--
ALTER TABLE `r_industries`
  ADD PRIMARY KEY (`iid`);

--
-- Indexes for table `r_investments`
--
ALTER TABLE `r_investments`
  ADD PRIMARY KEY (`vestid`),
  ADD KEY `coid` (`coid`),
  ADD KEY `proposedBy` (`proposedBy`);

--
-- Indexes for table `r_invites`
--
ALTER TABLE `r_invites`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `id` (`id`),
  ADD KEY `inviter` (`inviter`);

--
-- Indexes for table `r_invoices`
--
ALTER TABLE `r_invoices`
  ADD PRIMARY KEY (`nvid`),
  ADD KEY `payer` (`payer`),
  ADD KEY `payee` (`payee`),
  ADD KEY `created` (`created`),
  ADD KEY `status` (`status`);

--
-- Indexes for table `r_ips`
--
ALTER TABLE `r_ips`
  ADD PRIMARY KEY (`ip`),
  ADD KEY `uid` (`uid`);

--
-- Indexes for table `r_log`
--
ALTER TABLE `r_log`
  ADD PRIMARY KEY (`logid`),
  ADD KEY `type` (`type`),
  ADD KEY `channel` (`channel`),
  ADD KEY `myid` (`myid`),
  ADD KEY `agent` (`agent`);

--
-- Indexes for table `r_near`
--
ALTER TABLE `r_near`
  ADD PRIMARY KEY (`uid1`,`uid2`);

--
-- Indexes for table `r_nonmembers`
--
ALTER TABLE `r_nonmembers`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `r_notices`
--
ALTER TABLE `r_notices`
  ADD PRIMARY KEY (`msgid`),
  ADD KEY `uid` (`uid`);

--
-- Indexes for table `r_options`
--
ALTER TABLE `r_options`
  ADD PRIMARY KEY (`id`),
  ADD KEY `question` (`question`);

--
-- Indexes for table `r_pairs`
--
ALTER TABLE `r_pairs`
  ADD PRIMARY KEY (`id`),
  ADD KEY `option1` (`option1`),
  ADD KEY `option2` (`option2`);

--
-- Indexes for table `r_photos`
--
ALTER TABLE `r_photos`
  ADD PRIMARY KEY (`uid`);

--
-- Indexes for table `r_proposals`
--
ALTER TABLE `r_proposals`
  ADD PRIMARY KEY (`id`),
  ADD KEY `ctty` (`ctty`),
  ADD KEY `name` (`name`);

--
-- Indexes for table `r_proxies`
--
ALTER TABLE `r_proxies`
  ADD PRIMARY KEY (`id`),
  ADD KEY `person` (`person`);

--
-- Indexes for table `r_questions`
--
ALTER TABLE `r_questions`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `r_ratings`
--
ALTER TABLE `r_ratings`
  ADD PRIMARY KEY (`ratingid`),
  ADD KEY `vestid` (`vestid`),
  ADD KEY `uid` (`uid`);

--
-- Indexes for table `r_recurs`
--
ALTER TABLE `r_recurs`
  ADD PRIMARY KEY (`id`),
  ADD KEY `payer` (`payer`),
  ADD KEY `payee` (`payee`);

--
-- Indexes for table `r_regions`
--
ALTER TABLE `r_regions`
  ADD PRIMARY KEY (`region`),
  ADD UNIQUE KEY `fullName` (`fullName`),
  ADD KEY `state` (`st`);

--
-- Indexes for table `r_relations`
--
ALTER TABLE `r_relations`
  ADD PRIMARY KEY (`reid`),
  ADD KEY `main` (`main`),
  ADD KEY `other` (`other`);

--
-- Indexes for table `r_request`
--
ALTER TABLE `r_request`
  ADD PRIMARY KEY (`listid`),
  ADD KEY `ctty` (`ctty`);

--
-- Indexes for table `r_shares`
--
ALTER TABLE `r_shares`
  ADD PRIMARY KEY (`shid`),
  ADD KEY `vestid` (`vestid`);

--
-- Indexes for table `r_stakes`
--
ALTER TABLE `r_stakes`
  ADD PRIMARY KEY (`stakeid`),
  ADD KEY `uid` (`uid`);

--
-- Indexes for table `r_states`
--
ALTER TABLE `r_states`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `name_country_id` (`name`,`country_id`),
  ADD KEY `country_id` (`country_id`);

--
-- Indexes for table `r_stats`
--
ALTER TABLE `r_stats`
  ADD PRIMARY KEY (`id`),
  ADD KEY `ctty` (`ctty`);

--
-- Indexes for table `r_tous`
--
ALTER TABLE `r_tous`
  ADD PRIMARY KEY (`id`),
  ADD KEY `uid` (`uid`);

--
-- Indexes for table `r_transit`
--
ALTER TABLE `r_transit`
  ADD PRIMARY KEY (`location`);

--
-- Indexes for table `r_usd`
--
ALTER TABLE `r_usd`
  ADD PRIMARY KEY (`txid`),
  ADD KEY `created` (`created`);

--
-- Indexes for table `r_usd2`
--
ALTER TABLE `r_usd2`
  ADD PRIMARY KEY (`id`),
  ADD KEY `completed` (`completed`);

--
-- Indexes for table `r_user_industries`
--
ALTER TABLE `r_user_industries`
  ADD PRIMARY KEY (`id`),
  ADD KEY `uid` (`uid`),
  ADD KEY `iid` (`iid`);

--
-- Indexes for table `r_votes`
--
ALTER TABLE `r_votes`
  ADD PRIMARY KEY (`id`),
  ADD KEY `ballot` (`ballot`),
  ADD KEY `option` (`option`);

--
-- Indexes for table `semaphore`
--
ALTER TABLE `semaphore`
  ADD PRIMARY KEY (`name`),
  ADD KEY `value` (`value`),
  ADD KEY `expire` (`expire`);

--
-- Indexes for table `sessions`
--
ALTER TABLE `sessions`
  ADD PRIMARY KEY (`sid`,`ssid`),
  ADD KEY `timestamp` (`timestamp`),
  ADD KEY `uid` (`uid`),
  ADD KEY `ssid` (`ssid`);

--
-- Indexes for table `signup`
--
ALTER TABLE `signup`
  ADD PRIMARY KEY (`preid`);

--
-- Indexes for table `system`
--
ALTER TABLE `system`
  ADD PRIMARY KEY (`filename`),
  ADD KEY `system_list` (`status`,`bootstrap`,`type`,`weight`,`name`),
  ADD KEY `type_name` (`type`,`name`);

--
-- Indexes for table `tx_disputes_all`
--
ALTER TABLE `tx_disputes_all`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `tx_entries_all`
--
ALTER TABLE `tx_entries_all`
  ADD PRIMARY KEY (`id`),
  ADD KEY `xid` (`xid`),
  ADD KEY `uid` (`uid`);

--
-- Indexes for table `tx_hdrs_all`
--
ALTER TABLE `tx_hdrs_all`
  ADD PRIMARY KEY (`xid`),
  ADD UNIQUE KEY `xid` (`xid`),
  ADD UNIQUE KEY `reversesXid` (`reversesXid`),
  ADD KEY `actorId` (`actorId`),
  ADD KEY `created` (`created`);

--
-- Indexes for table `users`
--
ALTER TABLE `users`
  ADD PRIMARY KEY (`uid`),
  ADD KEY `access` (`access`),
  ADD KEY `created` (`created`),
  ADD KEY `mail` (`email`),
  ADD KEY `picture` (`picture`),
  ADD KEY `name` (`name`);

--
-- Indexes for table `variable`
--
ALTER TABLE `variable`
  ADD PRIMARY KEY (`name`);

--
-- Indexes for table `x_invoices`
--
ALTER TABLE `x_invoices`
  ADD PRIMARY KEY (`nvid`,`deleted`),
  ADD KEY `payer` (`payer`),
  ADD KEY `payee` (`payee`),
  ADD KEY `created` (`created`),
  ADD KEY `status` (`status`);

--
-- Indexes for table `x_photos`
--
ALTER TABLE `x_photos`
  ADD PRIMARY KEY (`uid`,`deleted`);

--
-- Indexes for table `x_relations`
--
ALTER TABLE `x_relations`
  ADD PRIMARY KEY (`reid`,`deleted`),
  ADD KEY `main` (`main`),
  ADD KEY `other` (`other`);

--
-- Indexes for table `x_txs`
--
ALTER TABLE `x_txs`
  ADD PRIMARY KEY (`xid`,`deleted`),
  ADD KEY `payer` (`payer`),
  ADD KEY `payee` (`payee`),
  ADD KEY `created` (`created`);

--
-- Indexes for table `x_usd`
--
ALTER TABLE `x_usd`
  ADD PRIMARY KEY (`txid`,`deleted`),
  ADD KEY `created` (`created`);

--
-- Indexes for table `x_users`
--
ALTER TABLE `x_users`
  ADD PRIMARY KEY (`uid`,`deleted`),
  ADD KEY `access` (`access`),
  ADD KEY `created` (`created`),
  ADD KEY `mail` (`email`),
  ADD KEY `picture` (`picture`),
  ADD KEY `name` (`name`);

DROP TABLE IF EXISTS `tx_disputes_deleted`, `tx_entries`, `tx_entries_deleted`, `tx_entries_payee`, `tx_entries_payer`, `tx_hdrs`, `tx_hdrs_deleted`;
ALTER TABLE `r_boxes` CHANGE `id` `id` BIGINT(20) NOT NULL AUTO_INCREMENT COMMENT 'device record id';
ALTER TABLE `r_changes` CHANGE `id` `id` BIGINT(20) NOT NULL AUTO_INCREMENT COMMENT 'change record ID';