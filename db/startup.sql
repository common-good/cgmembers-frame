-- Common Good database as of migration 20261107093900. Made by db/make-startup.sh; do not edit by hand.

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `admins`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `admins` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `uid` bigint(20) DEFAULT NULL COMMENT 'related account record ID',
  `vKeyE` longblob DEFAULT NULL COMMENT 'very secret private key (vKey) encrypted with a password specific to this account',
  `can` bigint(20) DEFAULT NULL COMMENT 'bit array of permissions for this admin',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_general_ci COMMENT='permissions for each admin';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `ancestors`;
/*!50001 DROP VIEW IF EXISTS `ancestors`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8mb4;
/*!50001 CREATE VIEW `ancestors` AS SELECT
 NULL AS `base`,
 NULL AS `baseIndustry`,
 NULL AS `ancestor`,
 NULL AS `ancestorIndustry` */;
SET character_set_client = @saved_cs_client;
DROP TABLE IF EXISTS `block`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `block` (
  `bid` int(11) NOT NULL AUTO_INCREMENT COMMENT 'Primary Key: Unique block ID.',
  `module` varchar(64) NOT NULL DEFAULT '' COMMENT 'The module from which the block originates; for example, ’user’ for the Who’s Online block, and ’block’ for any custom blocks.',
  `delta` varchar(32) NOT NULL DEFAULT '0' COMMENT 'Unique ID for block within a module.',
  `theme` varchar(64) NOT NULL DEFAULT '' COMMENT 'The theme under which the block settings apply.',
  `status` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'Block enabled status. (1 = enabled, 0 = disabled)',
  `weight` int(11) NOT NULL DEFAULT 0 COMMENT 'Block weight within region.',
  `region` varchar(64) NOT NULL DEFAULT '' COMMENT 'Theme region within which the block is set.',
  `custom` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'Flag to indicate how users may control visibility of the block. (0 = Users cannot control, 1 = On by default, but can be hidden, 2 = Hidden by default, but can be shown)',
  `visibility` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'Flag to indicate how to show blocks on pages. (0 = Show on all pages except listed pages, 1 = Show only on listed pages, 2 = Use custom PHP code to determine visibility)',
  `pages` mediumtext NOT NULL COMMENT 'Contents of the "Pages" block; contains either a list of paths on which to include/exclude the block or PHP code, depending on "visibility" setting.',
  `title` varchar(255) NOT NULL DEFAULT '' COMMENT 'Custom title for the block. (Empty string will use block default title, <none> will remove the title, text will cause block to use specified title.)',
  `cache` tinyint(4) NOT NULL DEFAULT 1 COMMENT 'Binary flag to indicate block cache mode. (-2: Custom cache, -1: Do not cache, 1: Cache per role, 2: Cache per user, 4: Cache per page, 8: Block cache global) See DRUPAL_CACHE_* constants in ../includes/common.inc for more detailed information.',
  PRIMARY KEY (`bid`),
  UNIQUE KEY `tmd` (`theme`,`module`,`delta`),
  KEY `list` (`theme`,`status`,`region`,`weight`,`module`)
) ENGINE=InnoDB AUTO_INCREMENT=427 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Stores block settings, such as region and visibility...';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `cache`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `cache` (
  `cid` varchar(255) NOT NULL DEFAULT '' COMMENT 'Primary Key: Unique cache ID.',
  `data` longblob DEFAULT NULL COMMENT 'A collection of data to cache.',
  `expire` int(11) NOT NULL DEFAULT 0 COMMENT 'A Unix timestamp indicating when the cache entry should expire, or 0 for never.',
  `created` int(11) NOT NULL DEFAULT 0 COMMENT 'A Unix timestamp indicating when the cache entry was created.',
  `serialized` smallint(6) NOT NULL DEFAULT 0 COMMENT 'A flag to indicate whether content is serialized (1) or not (0).',
  PRIMARY KEY (`cid`),
  KEY `expire` (`expire`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Generic cache table for caching things not separated out...';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `cache_bootstrap`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `cache_bootstrap` (
  `cid` varchar(255) NOT NULL DEFAULT '' COMMENT 'Primary Key: Unique cache ID.',
  `data` longblob DEFAULT NULL COMMENT 'A collection of data to cache.',
  `expire` int(11) NOT NULL DEFAULT 0 COMMENT 'A Unix timestamp indicating when the cache entry should expire, or 0 for never.',
  `created` int(11) NOT NULL DEFAULT 0 COMMENT 'A Unix timestamp indicating when the cache entry was created.',
  `serialized` smallint(6) NOT NULL DEFAULT 0 COMMENT 'A flag to indicate whether content is serialized (1) or not (0).',
  PRIMARY KEY (`cid`),
  KEY `expire` (`expire`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Cache table for data required to bootstrap Drupal, may be...';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `cache_form`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `cache_form` (
  `cid` varchar(255) NOT NULL DEFAULT '' COMMENT 'Primary Key: Unique cache ID.',
  `data` longblob DEFAULT NULL COMMENT 'A collection of data to cache.',
  `expire` int(11) NOT NULL DEFAULT 0 COMMENT 'A Unix timestamp indicating when the cache entry should expire, or 0 for never.',
  `created` int(11) NOT NULL DEFAULT 0 COMMENT 'A Unix timestamp indicating when the cache entry was created.',
  `serialized` smallint(6) NOT NULL DEFAULT 0 COMMENT 'A flag to indicate whether content is serialized (1) or not (0).',
  PRIMARY KEY (`cid`),
  KEY `expire` (`expire`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Cache table for the form system to store recently built...';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `cache_menu`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `cache_menu` (
  `cid` varchar(255) NOT NULL DEFAULT '' COMMENT 'Primary Key: Unique cache ID.',
  `data` longblob DEFAULT NULL COMMENT 'A collection of data to cache.',
  `expire` int(11) NOT NULL DEFAULT 0 COMMENT 'A Unix timestamp indicating when the cache entry should expire, or 0 for never.',
  `created` int(11) NOT NULL DEFAULT 0 COMMENT 'A Unix timestamp indicating when the cache entry was created.',
  `serialized` smallint(6) NOT NULL DEFAULT 0 COMMENT 'A flag to indicate whether content is serialized (1) or not (0).',
  PRIMARY KEY (`cid`),
  KEY `expire` (`expire`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Cache table for the menu system to store router...';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `changes`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `changes` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'change record ID',
  `table` varchar(255) DEFAULT NULL COMMENT 'table name',
  `rid` bigint(20) DEFAULT NULL COMMENT 'record ID of record that changed',
  `created` int(11) DEFAULT NULL COMMENT 'Unix date and time that change was made',
  `field` varchar(40) DEFAULT NULL COMMENT 'name of the field that was changed',
  `oldValue` blob DEFAULT NULL COMMENT 'serialized old value, possibly encrypted',
  `newValue` blob DEFAULT NULL COMMENT 'serialized new value, possibly encrypted',
  `changedBy` bigint(20) DEFAULT NULL COMMENT 'uid of user who made the change',
  PRIMARY KEY (`id`),
  KEY `rid` (`rid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Member record changes';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `cu_folders`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `cu_folders` (
  `id` bigint(20) NOT NULL COMMENT 'record ID',
  `name` varchar(255) DEFAULT NULL COMMENT 'folder name',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_general_ci COMMENT='groups of lists';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `cu_lists`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `cu_lists` (
  `id` bigint(20) NOT NULL COMMENT 'record ID',
  `name` varchar(255) DEFAULT NULL COMMENT 'list name',
  `folder` bigint(20) DEFAULT NULL COMMENT 'folder record ID, if any',
  `space` bigint(20) DEFAULT NULL COMMENT 'space record ID, if not in any folder',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_general_ci COMMENT='lists of tasks';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `cu_members`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `cu_members` (
  `id` bigint(20) NOT NULL COMMENT 'record ID',
  `uid` bigint(20) DEFAULT NULL COMMENT 'account record ID, if any',
  `name` varchar(255) DEFAULT NULL COMMENT 'team member name',
  `nick` varchar(5) DEFAULT NULL COMMENT 'team member nickname',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_general_ci COMMENT='clickup members and guests on the Common Good team';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `cu_spaces`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `cu_spaces` (
  `id` bigint(20) NOT NULL COMMENT 'record ID',
  `name` varchar(255) DEFAULT NULL COMMENT 'space name',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_general_ci COMMENT='top level categories';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `cu_tasks`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `cu_tasks` (
  `id` varchar(255) NOT NULL COMMENT 'record ID',
  `name` varchar(255) DEFAULT NULL COMMENT 'task name',
  `parent` varchar(255) DEFAULT NULL COMMENT 'parent task, if this is a subtask',
  `list` bigint(20) DEFAULT NULL COMMENT 'list record ID',
  `status` varchar(255) DEFAULT NULL COMMENT 'task status',
  `priority` varchar(255) DEFAULT NULL COMMENT 'task priority',
  `class` varchar(255) DEFAULT NULL COMMENT 'category',
  `tags` varchar(255) DEFAULT NULL COMMENT 'comma-delimted list of tags for this task',
  `estimate` bigint(20) DEFAULT NULL COMMENT 'estimated time to complete this task',
  `cap` bigint(20) DEFAULT NULL COMMENT 'maximum time to complete this task',
  `spent` bigint(20) DEFAULT NULL COMMENT 'time spent on this task',
  `closed` bigint(20) DEFAULT NULL COMMENT 'date/time task was closed',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_general_ci COMMENT='things to be done';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `cu_times`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `cu_times` (
  `id` bigint(20) NOT NULL COMMENT 'record ID',
  `task` varchar(255) DEFAULT NULL COMMENT 'task on which time was spent',
  `member` varchar(255) DEFAULT NULL COMMENT 'team member record ID',
  `start` bigint(20) DEFAULT NULL COMMENT 'date/time started',
  `stop` bigint(20) DEFAULT NULL COMMENT 'date/time stopped',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_general_ci COMMENT='amount of time spent on tasks';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `descendants`;
/*!50001 DROP VIEW IF EXISTS `descendants`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8mb4;
/*!50001 CREATE VIEW `descendants` AS SELECT
 NULL AS `base`,
 NULL AS `baseIndustry`,
 NULL AS `descendant`,
 NULL AS `descendantIndustry` */;
SET character_set_client = @saved_cs_client;
DROP TABLE IF EXISTS `flood`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `flood` (
  `fid` int(11) NOT NULL AUTO_INCREMENT COMMENT 'Unique flood event ID.',
  `event` varchar(64) NOT NULL DEFAULT '' COMMENT 'Name of event (e.g. contact).',
  `identifier` varchar(128) NOT NULL DEFAULT '' COMMENT 'Identifier of the visitor, such as an IP address or hostname.',
  `timestamp` int(11) NOT NULL DEFAULT 0 COMMENT 'Timestamp of the event.',
  `expiration` int(11) NOT NULL DEFAULT 0 COMMENT 'Expiration timestamp. Expired events are purged on cron run.',
  PRIMARY KEY (`fid`),
  KEY `allow` (`event`,`identifier`,`timestamp`),
  KEY `purge` (`expiration`)
) ENGINE=InnoDB AUTO_INCREMENT=2237 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Flood controls the threshold of events, such as the...';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `grants`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `grants` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `uid` bigint(20) DEFAULT NULL COMMENT 'record ID of the sponsee account',
  `pid` bigint(20) DEFAULT NULL COMMENT 'people record ID for the grantor',
  `fund` varchar(255) DEFAULT NULL COMMENT 'specific Fund within grant-making organization (if any)',
  `driveFileId` varchar(255) DEFAULT NULL COMMENT 'Google Drive file id of the uploaded grant agreement PDF (Phase 3.5 PR B)',
  `xid` bigint(20) DEFAULT NULL COMMENT 'transaction ID crediting the sponsee account',
  `amount` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'amount of expected grant',
  `by` enum('check','ach','wire') NOT NULL COMMENT 'how the funds are expected to be sent',
  `ckNum` varchar(255) DEFAULT NULL COMMENT 'grant check number, if any',
  `ckDate` bigint(20) DEFAULT NULL COMMENT 'grant check date, if any',
  `documented` bigint(20) DEFAULT NULL COMMENT 'date the grant agreement was sent to Common Good',
  `received` bigint(20) DEFAULT NULL COMMENT 'date the funds were received',
  `created` bigint(20) DEFAULT NULL COMMENT 'date the sponsee notified us',
  PRIMARY KEY (`id`),
  KEY `uid` (`uid`),
  KEY `pid` (`pid`),
  KEY `xid` (`xid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_general_ci COMMENT='record of grants expected by fiscally sponsored partners';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `menu_links`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `menu_links` (
  `menu_name` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT 'The menu name. All links with the same menu name (such as ’navigation’) are part of the same menu.',
  `mlid` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT 'The menu link ID (mlid) is the integer primary key.',
  `plid` int(10) unsigned NOT NULL DEFAULT 0 COMMENT 'The parent link ID (plid) is the mlid of the link above in the hierarchy, or zero if the link is at the top level in its menu.',
  `link_path` varchar(191) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT 'The Drupal path or external path this link points to.',
  `router_path` varchar(191) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT 'For links corresponding to a Drupal path (external = 0), this connects the link to a menu_router.path for joins.',
  `link_title` varchar(191) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT 'The text displayed for the link, which may be modified by a title callback stored in menu_router.',
  `options` blob DEFAULT NULL COMMENT 'A serialized array of options to be passed to the url() or l() function, such as a query string or HTML attributes.',
  `module` varchar(191) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT 'system' COMMENT 'The name of the module that generated this link.',
  `hidden` smallint(6) NOT NULL DEFAULT 0 COMMENT 'A flag for whether the link should be rendered in menus. (1 = a disabled menu item that may be shown on admin screens, -1 = a menu callback, 0 = a normal, visible link)',
  `external` smallint(6) NOT NULL DEFAULT 0 COMMENT 'A flag to indicate if the link points to a full URL starting with a protocol, like http:// (1 = external, 0 = internal).',
  `has_children` smallint(6) NOT NULL DEFAULT 0 COMMENT 'Flag indicating whether any links have this link as a parent (1 = children exist, 0 = no children).',
  `expanded` smallint(6) NOT NULL DEFAULT 0 COMMENT 'Flag for whether this link should be rendered as expanded in menus - expanded links always have their child links displayed, instead of only when the link is in the active trail (1 = expanded, 0 = not expanded)',
  `weight` int(11) NOT NULL DEFAULT 0 COMMENT 'Link weight among links in the same menu at the same depth.',
  `depth` smallint(6) NOT NULL DEFAULT 0 COMMENT 'The depth relative to the top level. A link with plid == 0 will have depth == 1.',
  `customized` smallint(6) NOT NULL DEFAULT 0 COMMENT 'A flag to indicate that the user has manually created or edited the link (1 = customized, 0 = not customized).',
  `p1` int(10) unsigned NOT NULL DEFAULT 0 COMMENT 'The first mlid in the materialized path. If N = depth, then pN must equal the mlid. If depth > 1 then p(N-1) must equal the plid. All pX where X > depth must equal zero. The columns p1 .. p9 are also called the parents.',
  `p2` int(10) unsigned NOT NULL DEFAULT 0 COMMENT 'The second mlid in the materialized path. See p1.',
  `p3` int(10) unsigned NOT NULL DEFAULT 0 COMMENT 'The third mlid in the materialized path. See p1.',
  `p4` int(10) unsigned NOT NULL DEFAULT 0 COMMENT 'The fourth mlid in the materialized path. See p1.',
  `p5` int(10) unsigned NOT NULL DEFAULT 0 COMMENT 'The fifth mlid in the materialized path. See p1.',
  `p6` int(10) unsigned NOT NULL DEFAULT 0 COMMENT 'The sixth mlid in the materialized path. See p1.',
  `p7` int(10) unsigned NOT NULL DEFAULT 0 COMMENT 'The seventh mlid in the materialized path. See p1.',
  `p8` int(10) unsigned NOT NULL DEFAULT 0 COMMENT 'The eighth mlid in the materialized path. See p1.',
  `p9` int(10) unsigned NOT NULL DEFAULT 0 COMMENT 'The ninth mlid in the materialized path. See p1.',
  `updated` smallint(6) NOT NULL DEFAULT 0 COMMENT 'Flag that indicates that this link was generated during the update from Drupal 5.',
  PRIMARY KEY (`mlid`),
  KEY `path_menu` (`link_path`(128),`menu_name`),
  KEY `menu_plid_expand_child` (`menu_name`,`plid`,`expanded`,`has_children`),
  KEY `menu_parents` (`menu_name`,`p1`,`p2`,`p3`,`p4`,`p5`,`p6`,`p7`,`p8`,`p9`),
  KEY `router_path` (`router_path`(128))
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_general_ci ROW_FORMAT=DYNAMIC COMMENT='Contains the individual links within a menu.';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `menu_router`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `menu_router` (
  `path` varchar(255) NOT NULL DEFAULT '' COMMENT 'Primary Key: the Drupal path this entry describes',
  `load_functions` blob NOT NULL COMMENT 'A serialized array of function names (like node_load) to be called to load an object corresponding to a part of the current path.',
  `to_arg_functions` blob NOT NULL COMMENT 'A serialized array of function names (like user_uid_optional_to_arg) to be called to replace a part of the router path with another string.',
  `access_callback` varchar(255) NOT NULL DEFAULT '' COMMENT 'The callback which determines the access to this router path. Defaults to user_access.',
  `access_arguments` blob DEFAULT NULL COMMENT 'A serialized array of arguments for the access callback.',
  `page_callback` varchar(255) NOT NULL DEFAULT '' COMMENT 'The name of the function that renders the page.',
  `page_arguments` blob DEFAULT NULL COMMENT 'A serialized array of arguments for the page callback.',
  `delivery_callback` varchar(255) NOT NULL DEFAULT '' COMMENT 'The name of the function that sends the result of the page_callback function to the browser.',
  `fit` int(11) NOT NULL DEFAULT 0 COMMENT 'A numeric representation of how specific the path is.',
  `number_parts` smallint(6) NOT NULL DEFAULT 0 COMMENT 'Number of parts in this router path.',
  `context` int(11) NOT NULL DEFAULT 0 COMMENT 'Only for local tasks (tabs) - the context of a local task to control its placement.',
  `tab_parent` varchar(255) NOT NULL DEFAULT '' COMMENT 'Only for local tasks (tabs) - the router path of the parent page (which may also be a local task).',
  `tab_root` varchar(255) NOT NULL DEFAULT '' COMMENT 'Router path of the closest non-tab parent page. For pages that are not local tasks, this will be the same as the path.',
  `title` varchar(255) NOT NULL DEFAULT '' COMMENT 'The title for the current page, or the title for the tab if this is a local task.',
  `title_callback` varchar(255) NOT NULL DEFAULT '' COMMENT 'A function which will alter the title. Defaults to t()',
  `title_arguments` varchar(255) NOT NULL DEFAULT '' COMMENT 'A serialized array of arguments for the title callback. If empty, the title will be used as the sole argument for the title callback.',
  `theme_callback` varchar(255) NOT NULL DEFAULT '' COMMENT 'A function which returns the name of the theme that will be used to render this page. If left empty, the default theme will be used.',
  `theme_arguments` varchar(255) NOT NULL DEFAULT '' COMMENT 'A serialized array of arguments for the theme callback.',
  `type` int(11) NOT NULL DEFAULT 0 COMMENT 'Numeric representation of the type of the menu item, like MENU_LOCAL_TASK.',
  `description` mediumtext NOT NULL COMMENT 'A description of this item.',
  `position` varchar(255) NOT NULL DEFAULT '' COMMENT 'The position of the block (left or right) on the system administration page for this item.',
  `weight` int(11) NOT NULL DEFAULT 0 COMMENT 'Weight of the element. Lighter weights are higher up, heavier weights go down.',
  `include_file` longtext DEFAULT NULL COMMENT 'The file to include for this element, usually the page callback function lives in this file.',
  PRIMARY KEY (`path`),
  KEY `fit` (`fit`),
  KEY `tab_parent` (`tab_parent`(64),`weight`,`title`),
  KEY `tab_root_weight_title` (`tab_root`(64),`weight`,`title`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Maps paths to various callbacks (access, page and title)';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `messages`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `messages` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `postid` bigint(20) DEFAULT NULL COMMENT 'related post ID',
  `message` varchar(255) DEFAULT NULL COMMENT 'the message',
  `sender` bigint(20) DEFAULT NULL COMMENT 'pid of sender',
  `confirmed` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'confirmed by email',
  `created` int(11) DEFAULT NULL COMMENT 'creation date',
  PRIMARY KEY (`id`),
  KEY `sender` (`sender`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='messages responding to offers and needs';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `ofac`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `ofac` (
  `nm` varchar(255) DEFAULT NULL COMMENT 'the person''s name',
  `co` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'is this a company?',
  KEY `nm` (`nm`),
  KEY `co` (`co`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_general_ci COMMENT='list of known foreign criminals';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `people`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `people` (
  `pid` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'record ID',
  `uid` bigint(20) DEFAULT NULL COMMENT 'poster''s associated account ID, if any',
  `displayName` varchar(255) DEFAULT NULL COMMENT 'first name or nickname',
  `fullName` varchar(255) DEFAULT NULL COMMENT 'full name',
  `street` varchar(255) DEFAULT NULL COMMENT 'street address without street number or apt number',
  `address` varchar(255) DEFAULT NULL COMMENT 'physical address',
  `city` varchar(255) DEFAULT NULL COMMENT 'city',
  `state` mediumint(9) DEFAULT NULL COMMENT 'state',
  `zip` varchar(255) DEFAULT NULL COMMENT 'postal code',
  `country` mediumint(8) NOT NULL DEFAULT 1228 COMMENT 'country index',
  `phone` varchar(255) DEFAULT NULL COMMENT 'phone number',
  `email` varchar(255) DEFAULT NULL COMMENT 'email address',
  `method` enum('email','phone','text') DEFAULT NULL COMMENT 'preferred contact method',
  `latitude` decimal(11,8) NOT NULL DEFAULT 0.00000000 COMMENT 'latitude of location',
  `longitude` decimal(11,8) NOT NULL DEFAULT 0.00000000 COMMENT 'longitude of location',
  `notes` longtext DEFAULT NULL COMMENT 'miscellaneous notes about the person',
  `confirmed` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'confirmed by email',
  `created` int(11) DEFAULT NULL COMMENT 'creation date',
  `notices` text DEFAULT NULL COMMENT 'notice preferences',
  `health` varchar(255) NOT NULL DEFAULT '0' COMMENT 'summary of COVID19 survey answers',
  `source` mediumtext DEFAULT NULL COMMENT 'how did this person hear about us?',
  `stripeCid` varchar(255) DEFAULT NULL COMMENT 'customerId from Stripe',
  PRIMARY KEY (`pid`),
  KEY `email` (`email`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='contact information for non-members';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `phinxlog`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `phinxlog` (
  `version` bigint(20) NOT NULL,
  `migration_name` varchar(100) DEFAULT NULL,
  `start_time` timestamp NULL DEFAULT NULL,
  `end_time` timestamp NULL DEFAULT NULL,
  `breakpoint` tinyint(1) NOT NULL DEFAULT 0,
  PRIMARY KEY (`version`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC;
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `post_cats`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `post_cats` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `cat` varchar(255) DEFAULT NULL COMMENT 'category of service',
  `sort` mediumint(9) NOT NULL DEFAULT 0 COMMENT 'sorting order',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=22 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='types of offers and needs';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `posts`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `posts` (
  `postid` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'record ID',
  `type` enum('need','offer','tip') DEFAULT NULL COMMENT 'item type',
  `item` varchar(255) DEFAULT NULL COMMENT 'item offered or needed',
  `details` longtext DEFAULT NULL COMMENT 'description of item',
  `cat` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'service category',
  `service` tinyint(4) DEFAULT NULL COMMENT '0=goods 1=service',
  `exchange` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'is exchange wanted?',
  `emergency` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'because of a short-term emergency',
  `radius` decimal(11,6) DEFAULT NULL COMMENT 'geographic limit of post visibility, in miles',
  `pid` bigint(20) DEFAULT NULL COMMENT 'poster''s associated people record ID, if any',
  `hits` bigint(20) NOT NULL DEFAULT 0 COMMENT 'number of times details about this item have been viewed',
  `contacts` bigint(20) NOT NULL DEFAULT 0 COMMENT 'number of contacts about this item',
  `confirmed` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'confirmed by email',
  `created` int(11) DEFAULT NULL COMMENT 'start date',
  `end` int(11) DEFAULT NULL COMMENT 'end date',
  `private` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'show this post only to the administrator',
  PRIMARY KEY (`postid`),
  KEY `pid` (`pid`),
  KEY `type` (`type`),
  KEY `cat` (`cat`),
  KEY `created` (`created`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='offers and needs posted by members and non-members';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `queue`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `queue` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'primary key: Unique item ID',
  `item` longblob DEFAULT NULL COMMENT 'arbitrary data for the item.',
  `created` int(11) DEFAULT NULL COMMENT 'Unixtime record was created',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Cron queue';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `r_areas`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `r_areas` (
  `area_code` char(3) NOT NULL DEFAULT '' COMMENT 'telephone area code',
  `region` varchar(24) DEFAULT NULL COMMENT 'state, province, or territory',
  PRIMARY KEY (`area_code`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC;
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `r_bad`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `r_bad` (
  `qid` varchar(255) DEFAULT NULL COMMENT 'phoney customer qid',
  `code` varchar(255) DEFAULT NULL COMMENT 'phoney card security code',
  `created` int(11) NOT NULL COMMENT 'Unixtime record was created',
  PRIMARY KEY (`created`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='lost, stolen, or faked rCard codes';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `r_ballots`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `r_ballots` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'ballot record id',
  `question` bigint(20) NOT NULL DEFAULT 0 COMMENT 'question or proposal voted on by a particular voter',
  `voter` bigint(20) NOT NULL DEFAULT 0 COMMENT 'record id of voter whose ballot this is',
  `proxy` bigint(20) NOT NULL DEFAULT 0 COMMENT 'record id of voter who actually voted on behalf of the voter',
  `modified` int(11) NOT NULL DEFAULT 0 COMMENT 'date/time last modified',
  `created` int(11) NOT NULL DEFAULT 0 COMMENT 'date/time created',
  PRIMARY KEY (`id`),
  KEY `question` (`question`),
  KEY `voter` (`voter`),
  KEY `proxy` (`proxy`)
) ENGINE=InnoDB AUTO_INCREMENT=257489 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='A votable question addressed by a particular voter';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `r_banks`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `r_banks` (
  `route` int(9) NOT NULL COMMENT 'routing number',
  `branch` tinyint(4) DEFAULT NULL COMMENT 'is this a branch office',
  `fedRoute` int(9) DEFAULT NULL COMMENT 'routing number of servicing Fed',
  `type` tinyint(4) DEFAULT NULL COMMENT 'bank type',
  `modified` char(6) DEFAULT NULL COMMENT 'date modified',
  `newRoute` int(9) DEFAULT NULL COMMENT 'new routing number',
  `name` varchar(36) DEFAULT NULL COMMENT 'bank name',
  `address` varchar(36) DEFAULT NULL COMMENT 'bank address',
  `city` varchar(20) DEFAULT NULL COMMENT 'bank city',
  `state` char(2) DEFAULT NULL COMMENT 'bank state',
  `zip` char(9) DEFAULT NULL COMMENT 'bank zipcode',
  `phone` char(10) DEFAULT NULL COMMENT 'bank phone',
  `status` char(1) DEFAULT NULL COMMENT 'status',
  `view` char(1) DEFAULT NULL COMMENT 'status',
  PRIMARY KEY (`route`),
  KEY `newroute` (`newRoute`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Bank routing numbers';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `r_boxes`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `r_boxes` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'device record id',
  `channel` tinyint(4) DEFAULT NULL COMMENT 'channel',
  `code` varchar(255) DEFAULT NULL COMMENT 'device id',
  `boxnum` int(11) NOT NULL DEFAULT 0 COMMENT 'sequential device number for the account',
  `uid` bigint(20) DEFAULT NULL COMMENT 'account record id',
  `boxName` varchar(255) DEFAULT NULL COMMENT 'member''s chosen name for this device, for this account',
  `todo` longtext DEFAULT NULL COMMENT 'waiting for confirmation to complete this operation',
  `nonce` varchar(255) DEFAULT NULL COMMENT 'waiting for this nonce, for confirmation',
  `version` mediumint(9) DEFAULT NULL COMMENT 'latest software version on the device',
  `access` int(11) NOT NULL DEFAULT 0 COMMENT 'date/time last used',
  `created` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime record was created',
  `restricted` tinyint(4) DEFAULT 0 COMMENT 'permit no new users of this device',
  PRIMARY KEY (`id`),
  KEY `uid` (`uid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Names for devices';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `r_countries`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `r_countries` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT 'Country Id',
  `name` varchar(64) DEFAULT NULL COMMENT 'Country Name',
  `iso_code` char(2) DEFAULT NULL COMMENT 'ISO Code',
  `country_code` varchar(4) DEFAULT NULL COMMENT 'National prefix to be used when dialing TO this country.',
  `address_format_id` int(10) unsigned DEFAULT NULL COMMENT 'Foreign key to civicrm_address_format.id.',
  `idd_prefix` varchar(4) DEFAULT NULL COMMENT 'International direct dialing prefix from within the country TO another country',
  `ndd_prefix` varchar(4) DEFAULT NULL COMMENT 'Access prefix to call within a country to a different area',
  `region_id` int(10) unsigned NOT NULL COMMENT 'Foreign key to civicrm_worldregion.id.',
  `is_province_abbreviated` tinyint(4) DEFAULT 0 COMMENT 'Should state/province be displayed as abbreviation?',
  PRIMARY KEY (`id`),
  UNIQUE KEY `name_iso_code` (`name`,`iso_code`),
  KEY `address_format_id` (`address_format_id`),
  KEY `region_id` (`region_id`)
) ENGINE=InnoDB AUTO_INCREMENT=1247 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC;
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `r_criteria`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `r_criteria` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'criterion record id',
  `name` text DEFAULT NULL COMMENT 'name of criterion',
  `text` text DEFAULT NULL COMMENT 'text of the criterion',
  `detail` longtext DEFAULT NULL COMMENT 'additional detail about the criterion',
  `points` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'how many points for this criterion',
  `auto` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'is this criterion calculated automatically?',
  `displayOrder` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'where to display this criterion in the order',
  `ctty` bigint(20) NOT NULL DEFAULT 0 COMMENT 'community or region record id (zero for all)',
  `modified` int(11) NOT NULL DEFAULT 0 COMMENT 'date/time last modified',
  `created` int(11) NOT NULL DEFAULT 0 COMMENT 'date/time created',
  PRIMARY KEY (`id`),
  KEY `ctty` (`ctty`)
) ENGINE=InnoDB AUTO_INCREMENT=15 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Criteria for funding proposals';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `r_do`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `r_do` (
  `doid` int(11) NOT NULL AUTO_INCREMENT COMMENT 'record id',
  `expires` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime expiration',
  `data` longtext DEFAULT NULL COMMENT 'serialized array of parameters',
  `uid` bigint(20) NOT NULL DEFAULT 0 COMMENT 'related account record ID',
  `box` bigint(20) DEFAULT NULL COMMENT 'machine id',
  `created` int(11) DEFAULT 0 COMMENT 'Unixtime record was created',
  `completed` int(11) DEFAULT 0 COMMENT 'Unixtime action was completed',
  `action` longtext DEFAULT NULL COMMENT 'serialized array describing action to take',
  `code` varchar(255) DEFAULT NULL COMMENT 'security code',
  `before` int(11) DEFAULT 0 COMMENT 'Unixtime expiration',
  PRIMARY KEY (`doid`),
  KEY `uid` (`uid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Clickable actions with no signin';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `r_events`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `r_events` (
  `id` int(11) NOT NULL AUTO_INCREMENT COMMENT 'record id',
  `ctty` bigint(20) DEFAULT NULL COMMENT 'what community the event is in',
  `type` char(1) DEFAULT NULL COMMENT 'event type (I=in person, V=vote, G=grading, P=RFP)',
  `event` varchar(255) DEFAULT NULL COMMENT 'name of event',
  `details` longtext DEFAULT NULL COMMENT 'event details',
  `start` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime event begins',
  `end` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime event ends',
  PRIMARY KEY (`id`),
  KEY `ctty` (`ctty`)
) ENGINE=InnoDB AUTO_INCREMENT=8 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Events in each community’s democratic process';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `r_honors`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `r_honors` (
  `id` int(11) NOT NULL AUTO_INCREMENT COMMENT 'record id',
  `uid` bigint(20) DEFAULT NULL COMMENT 'uid of account making the gift to Common Good',
  `honor` varchar(10) DEFAULT NULL COMMENT 'what type of honor',
  `honored` longtext DEFAULT NULL COMMENT 'who is honored',
  `created` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime of first associated gift',
  PRIMARY KEY (`id`),
  KEY `uid` (`uid`)
) ENGINE=InnoDB AUTO_INCREMENT=270 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='gifts in honor or memory';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `r_industries`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `r_industries` (
  `iid` int(11) NOT NULL AUTO_INCREMENT,
  `industry` varchar(255) NOT NULL,
  `parent` int(11) DEFAULT NULL,
  PRIMARY KEY (`iid`),
  KEY `parent` (`parent`)
) ENGINE=InnoDB AUTO_INCREMENT=79 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC;
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `r_investments`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `r_investments` (
  `vestid` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'record ID',
  `coid` bigint(20) DEFAULT NULL COMMENT 'member company record ID',
  `clubid` bigint(20) DEFAULT NULL COMMENT 'investment club record ID',
  `proposedBy` bigint(20) DEFAULT NULL COMMENT 'account record ID of proposer',
  `investment` longtext DEFAULT NULL COMMENT 'description of investment',
  `return` decimal(7,6) DEFAULT NULL COMMENT 'predicted or actual APR',
  `types` varchar(4) DEFAULT NULL COMMENT 'D=dividends I=interest T=tax-exempt interest',
  `terms` mediumtext DEFAULT NULL COMMENT 'investment terms',
  `assets` decimal(11,2) DEFAULT NULL COMMENT 'company assets, bond, or collateral',
  `offering` decimal(11,2) DEFAULT NULL COMMENT 'size of offering',
  `price` decimal(11,2) DEFAULT NULL COMMENT 'price per share',
  `character` longtext DEFAULT NULL COMMENT 'assessment of the integrity and determination of the owners',
  `strength` tinyint(4) DEFAULT NULL COMMENT 'company''s financial strength (0 to 100)',
  `web` tinyint(4) DEFAULT NULL COMMENT 'impression on the web (0 to 100)',
  `history` tinyint(4) DEFAULT NULL COMMENT 'past repayment success (0 to 100)',
  `soundness` tinyint(4) DEFAULT NULL COMMENT 'overall how sound is this investment (0 to 100)',
  `reserve` decimal(10,3) NOT NULL DEFAULT 0.000 COMMENT 'fraction to hold in reserve for possible loss',
  PRIMARY KEY (`vestid`),
  KEY `coid` (`coid`),
  KEY `proposedBy` (`proposedBy`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='potential and actual investments';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `r_invites`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `r_invites` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'record id',
  `code` varchar(64) DEFAULT NULL COMMENT 'secret invitation code',
  `email` varchar(255) DEFAULT NULL COMMENT 'email of invitee',
  `inviter` bigint(20) NOT NULL DEFAULT 0 COMMENT 'uid of inviting member',
  `invitee` bigint(20) NOT NULL DEFAULT 0 COMMENT 'uid of invited new member',
  `invited` int(11) NOT NULL DEFAULT 0 COMMENT 'date of invitation',
  `subject` varchar(255) DEFAULT NULL COMMENT 'email subject',
  `message` longtext DEFAULT NULL COMMENT 'email message body',
  `zip` varchar(10) NOT NULL DEFAULT 'NULL' COMMENT 'alleged postal code of recipient',
  `nonudge` int(11) DEFAULT NULL COMMENT 'date/time this invitee "unsubscribed"',
  PRIMARY KEY (`id`),
  UNIQUE KEY `id` (`id`),
  KEY `inviter` (`inviter`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Who invited whom';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `r_ips`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `r_ips` (
  `ip` varchar(39) NOT NULL COMMENT 'ip address',
  `uid` bigint(20) DEFAULT NULL COMMENT 'account record ID',
  `device` varchar(255) DEFAULT NULL COMMENT 'device code',
  PRIMARY KEY (`ip`),
  KEY `uid` (`uid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='IP addresses of approved accounts';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `r_near`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `r_near` (
  `uid1` bigint(20) NOT NULL COMMENT 'account record ID of one account',
  `uid2` bigint(20) NOT NULL COMMENT 'account record ID of other account',
  `weight` mediumint(9) DEFAULT NULL COMMENT 'number of connections',
  PRIMARY KEY (`uid1`,`uid2`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='How members are connected';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `r_notices`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `r_notices` (
  `msgid` int(11) NOT NULL AUTO_INCREMENT COMMENT 'notice record id',
  `uid` bigint(20) DEFAULT NULL COMMENT 'account record ID of member notified',
  `created` int(11) NOT NULL DEFAULT 0 COMMENT 'date of notice',
  `type` varchar(255) DEFAULT NULL COMMENT 'type of notice',
  `sent` int(11) NOT NULL DEFAULT 0 COMMENT 'date sent (0 if not sent yet)',
  `message` longtext DEFAULT NULL COMMENT 'the notice text',
  PRIMARY KEY (`msgid`),
  KEY `uid` (`uid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Message digest buffer';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `r_options`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `r_options` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'option record id',
  `question` bigint(20) NOT NULL DEFAULT 0 COMMENT 'question for which this is an option',
  `text` text DEFAULT NULL COMMENT 'text of the option',
  `detail` longtext DEFAULT NULL COMMENT 'additional detail about the option',
  `displayOrder` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'where to display this option in the order',
  `minimum` bigint(20) NOT NULL DEFAULT 0 COMMENT 'the least (money) to budget for this option',
  `maximum` bigint(20) NOT NULL DEFAULT 0 COMMENT 'the most (money) to budget for this option',
  `mandatory` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'is the minimum required?',
  `averageGrade` decimal(5,3) NOT NULL DEFAULT 0.000 COMMENT 'average grade (in a penny vote, the fraction of all votes)',
  `averageMax` decimal(5,3) NOT NULL DEFAULT 0.000 COMMENT 'average maximum grade (for range votes)',
  `vetoes` int(11) NOT NULL DEFAULT 0 COMMENT 'number of vetoes for this option',
  `modified` int(11) NOT NULL DEFAULT 0 COMMENT 'date/time last modified',
  `created` int(11) NOT NULL DEFAULT 0 COMMENT 'date/time created',
  PRIMARY KEY (`id`),
  KEY `question` (`question`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Options for a question to be voted on';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `r_pairs`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `r_pairs` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'pairs record id',
  `option1` bigint(20) NOT NULL DEFAULT 0 COMMENT 'record id of one option',
  `option2` bigint(20) NOT NULL DEFAULT 0 COMMENT 'record id of the other option',
  `prefer1` int(11) NOT NULL DEFAULT 0 COMMENT 'how many voters prefer the option1',
  `prefer2` int(11) NOT NULL DEFAULT 0 COMMENT 'how many voters prefer the option2',
  `nopreference` int(11) NOT NULL DEFAULT 0 COMMENT 'how many voters had no preference between the two options',
  `raw` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'true if this record is calculated without counting proxies',
  `created` int(11) NOT NULL DEFAULT 0 COMMENT 'date/time created',
  PRIMARY KEY (`id`),
  KEY `option1` (`option1`),
  KEY `option2` (`option2`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Counts of preferences of one option over another';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `r_proposals`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `r_proposals` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'proposal record id',
  `event` bigint(20) DEFAULT NULL COMMENT 'what event this question is part of',
  `project` varchar(255) DEFAULT NULL COMMENT 'project title',
  `overview` longtext DEFAULT NULL COMMENT 'project overview',
  `categories` text DEFAULT NULL COMMENT 'funding categories (space-separated list)',
  `purpose` longtext DEFAULT NULL COMMENT 'why the project is needed',
  `systemic` longtext DEFAULT NULL COMMENT 'how the project promotes systemic change',
  `where` text DEFAULT NULL COMMENT 'zipcode where the project will take place',
  `when` int(11) DEFAULT NULL COMMENT 'project start date',
  `until` int(11) DEFAULT NULL COMMENT 'project end date',
  `how` longtext DEFAULT NULL COMMENT 'how the project will be accomplished',
  `amount` decimal(11,2) DEFAULT NULL COMMENT 'amount of funding requested',
  `type` tinyint(4) DEFAULT NULL COMMENT 'type of funding',
  `recovery` mediumint(9) NOT NULL DEFAULT 0 COMMENT 'how funds will return to the Community Fund',
  `budgetTotal` decimal(11,2) DEFAULT NULL COMMENT 'total expense budget',
  `budget` longtext DEFAULT NULL COMMENT 'detailed project budget',
  `committed` decimal(11,2) DEFAULT NULL COMMENT 'how much of the budget has already been raised',
  `contingency` longtext DEFAULT NULL COMMENT 'how the project organizers will cope with less funding than requested',
  `qualifications` longtext DEFAULT NULL COMMENT 'qualifications of project staff',
  `evaluation` longtext DEFAULT NULL COMMENT 'how the success of the project will be evaluated',
  `name` varchar(255) DEFAULT NULL COMMENT 'individual or organization making the proposal',
  `contact` varchar(255) DEFAULT NULL COMMENT 'contact person (or "self")',
  `phone` varchar(255) DEFAULT NULL COMMENT 'contact phone',
  `email` varchar(255) DEFAULT NULL COMMENT 'contact email',
  `sponsor` text DEFAULT NULL COMMENT 'member(s) sponsoring the project proposal',
  `ctty` bigint(20) DEFAULT NULL COMMENT 'community or region record id',
  PRIMARY KEY (`id`),
  KEY `ctty` (`ctty`),
  KEY `name` (`name`)
) ENGINE=InnoDB AUTO_INCREMENT=66 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='rCredits funding proposals';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `r_proxies`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `r_proxies` (
  `id` int(11) NOT NULL AUTO_INCREMENT COMMENT 'record id',
  `person` bigint(20) DEFAULT NULL COMMENT 'account record id',
  `proxy` bigint(20) DEFAULT NULL COMMENT 'account record id of proxy',
  `priority` tinyint(4) DEFAULT NULL COMMENT 'precedence of this proxy (1=top priority)',
  PRIMARY KEY (`id`),
  KEY `person` (`person`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Who represents whom';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `r_questions`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `r_questions` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'question record id',
  `event` bigint(20) DEFAULT NULL COMMENT 'what event this question is part of',
  `repeats` bigint(20) NOT NULL DEFAULT 0 COMMENT 'pointer to question that this is a revote on (0=none)',
  `repeatedBy` bigint(20) NOT NULL DEFAULT 0 COMMENT 'pointer to question that is a revote of this one (0=none))',
  `text` longtext DEFAULT NULL COMMENT 'text of the question',
  `detail` longtext DEFAULT NULL COMMENT 'additional detail about the question',
  `linkDiscussion` text DEFAULT NULL COMMENT 'link to online discussion of the issue',
  `type` char(1) NOT NULL DEFAULT 'M' COMMENT 'vote type M=multiple choice, B=budget (penny vote), R=range, E=essay',
  `units` varchar(255) DEFAULT NULL COMMENT 'budget units (defaults to money, measured in the community''s national currency)',
  `budget` decimal(11,0) NOT NULL DEFAULT 0 COMMENT 'how much (money) is to be budgeted',
  `minVeto` decimal(5,3) DEFAULT NULL COMMENT 'minimum veto fraction of vote, to force reconsideration',
  `optOrder` char(1) NOT NULL DEFAULT 'S' COMMENT 'option order',
  `voteCount` int(11) NOT NULL DEFAULT 0 COMMENT 'total number of votes',
  `result` longtext DEFAULT NULL COMMENT 'results of the vote or grading',
  `created` int(11) NOT NULL DEFAULT 0 COMMENT 'date/time created',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=14 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Questions to be voted on';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `r_ratings`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `r_ratings` (
  `ratingid` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'record ID',
  `vestid` int(11) DEFAULT NULL COMMENT 'investment record ID',
  `uid` bigint(20) DEFAULT NULL COMMENT 'member record ID',
  `good` tinyint(4) DEFAULT NULL COMMENT 'how well this investment serves the common good (0-100)',
  `recurs` decimal(11,2) DEFAULT NULL COMMENT 'how much the member will spend here monthly',
  `comment` longtext DEFAULT NULL COMMENT 'description of investment',
  `patronage` decimal(11,2) DEFAULT NULL COMMENT 'how much the member will spend here monthly',
  PRIMARY KEY (`ratingid`),
  KEY `vestid` (`vestid`),
  KEY `uid` (`uid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='how a member rates an investment';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `r_regions`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `r_regions` (
  `region` char(3) NOT NULL DEFAULT '' COMMENT 'region id',
  `fullName` varchar(255) DEFAULT NULL,
  `st` char(2) DEFAULT NULL COMMENT 'state or province abbreviation',
  `zips` text DEFAULT NULL COMMENT 'zip regex for this geographic region',
  `postalAddr` varchar(255) DEFAULT NULL,
  `federalId` varchar(9) NOT NULL,
  `hasServer` tinyint(1) NOT NULL DEFAULT 0,
  PRIMARY KEY (`region`),
  UNIQUE KEY `fullName` (`fullName`),
  KEY `state` (`st`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC;
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `r_shares`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `r_shares` (
  `shid` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'record ID',
  `vestid` int(11) DEFAULT NULL COMMENT 'investment record ID',
  `shares` int(11) NOT NULL DEFAULT 0 COMMENT 'club shares in the investment bought or (if <0) sold',
  `pending` int(11) NOT NULL DEFAULT 0 COMMENT 'number of shares to buy or (if <0) sell ASAP',
  `when` int(11) DEFAULT NULL COMMENT 'Unixtime investment made',
  `sold` int(11) DEFAULT NULL COMMENT 'Unixtime investment totally sold',
  PRIMARY KEY (`shid`),
  KEY `vestid` (`vestid`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='club stakes in investments';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `r_stakes`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `r_stakes` (
  `stakeid` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'record ID',
  `uid` bigint(20) NOT NULL DEFAULT 0 COMMENT 'member record ID',
  `clubid` bigint(20) NOT NULL DEFAULT 0 COMMENT 'investment club record ID',
  `stake` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'member stake in the club',
  `request` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'request to change stake by this amount',
  `joined` int(11) NOT NULL DEFAULT 0 COMMENT 'when this member joined the club',
  `requestedOut` int(11) NOT NULL DEFAULT 0 COMMENT 'when this member last requested cash out',
  PRIMARY KEY (`stakeid`),
  KEY `uid` (`uid`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='member stakes in an investment club';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `r_states`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `r_states` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT 'State / Province ID',
  `name` varchar(64) DEFAULT NULL COMMENT 'Name of State / Province',
  `abbreviation` varchar(4) DEFAULT NULL COMMENT '2-4 Character Abbreviation of State / Province',
  `country_id` int(10) unsigned NOT NULL COMMENT 'ID of Country that State / Province belongs to',
  PRIMARY KEY (`id`),
  UNIQUE KEY `name_country_id` (`name`,`country_id`),
  KEY `country_id` (`country_id`)
) ENGINE=InnoDB AUTO_INCREMENT=10057 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC;
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `r_stats`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `r_stats` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'statistics record id',
  `created` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime record was created',
  `ctty` bigint(20) DEFAULT NULL COMMENT 'community or region record id',
  `pAccts` mediumint(9) NOT NULL DEFAULT 0 COMMENT 'number of personal accounts',
  `bAccts` mediumint(9) NOT NULL DEFAULT 0 COMMENT 'number of company accounts',
  `newbs` mediumint(9) NOT NULL DEFAULT 0 COMMENT 'number of not-yet-active accounts',
  `aAccts` mediumint(9) NOT NULL DEFAULT 0 COMMENT 'number of active personal accounts',
  `conx` decimal(10,3) NOT NULL DEFAULT 0.000 COMMENT 'number of connections per personal account',
  `conxLocal` decimal(10,3) NOT NULL DEFAULT 0.000 COMMENT 'number of local connections per personal account',
  `balsPos` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'amount of positive balances',
  `balsNeg` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'amount of negative balances',
  `balsPosCount` mediumint(9) NOT NULL DEFAULT 0 COMMENT 'number of positive balances',
  `balsNegCount` mediumint(9) NOT NULL DEFAULT 0 COMMENT 'number of negative balances',
  `topN` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'top N or N% of balances, whichever is greater',
  `botN` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'bottom N or N% of balances, whichever is less',
  `floors` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'credit lines',
  `p2b` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'customer purchase volume',
  `b2b` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'b2b purchase volume',
  `b2p` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'payroll volume',
  `p2p` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'person-to-person purchase volume',
  `p2bCount` mediumint(9) NOT NULL DEFAULT 0 COMMENT 'customer purchase volume',
  `b2bCount` mediumint(9) NOT NULL DEFAULT 0 COMMENT 'b2b purchase volume',
  `b2pCount` mediumint(9) NOT NULL DEFAULT 0 COMMENT 'payroll volume',
  `p2pCount` mediumint(9) NOT NULL DEFAULT 0 COMMENT 'person-to-person purchase volume',
  `cashs` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'amount of exchanges for cash',
  `cashsCount` mediumint(9) NOT NULL DEFAULT 0 COMMENT 'number of exchanges for cash',
  `cgIn` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'amount of Common Good Credits coming into this community',
  `cgOut` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'amount of Common Good Credits leaving this community',
  `cgInCount` mediumint(9) NOT NULL DEFAULT 0 COMMENT 'number of transfers into this community',
  `cgOutCount` mediumint(9) NOT NULL DEFAULT 0 COMMENT 'number of transfers out of this community',
  `usdIn` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'amount of US Dollars brought into the system',
  `usdOut` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'amount of US Dollars taken out of the system',
  `usdInCount` mediumint(9) NOT NULL DEFAULT 0 COMMENT 'number of incoming bank transfers',
  `usdOutCount` mediumint(9) NOT NULL DEFAULT 0 COMMENT 'number of outgoing bank transfers',
  `payees` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'median number of payees per active account in the recent past',
  `basket` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'median (positive) amount per transaction in the recent past',
  `patronage` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'current intended recurring donations per month',
  `roundups` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'average roundups per month in the recent past',
  `crumbs` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'average crumbs per month in the recent past',
  `invites` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'total invitations to date',
  PRIMARY KEY (`id`),
  KEY `ctty` (`ctty`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Operating statistics for communities and overall';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `r_tous`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `r_tous` (
  `id` int(11) NOT NULL AUTO_INCREMENT COMMENT 'vote record id',
  `uid` bigint(20) DEFAULT NULL COMMENT 'account record ID',
  `time` int(11) NOT NULL DEFAULT 0 COMMENT 'date/time of message',
  `message` blob DEFAULT NULL COMMENT 'the message',
  PRIMARY KEY (`id`),
  KEY `uid` (`uid`)
) ENGINE=InnoDB AUTO_INCREMENT=66 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Secure messages sent from member to us.';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `r_transit`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `r_transit` (
  `id` int(11) NOT NULL,
  `location` varchar(255) NOT NULL,
  PRIMARY KEY (`location`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC;
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `r_usd2`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `r_usd2` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'record ID',
  `type` char(1) DEFAULT NULL COMMENT 'transaction type (S=service charge, T=transfer between accounts)',
  `amount` decimal(11,2) DEFAULT NULL COMMENT 'amount of transfer',
  `completed` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime transaction was completed',
  `bankTxId` bigint(20) NOT NULL DEFAULT 0 COMMENT 'bank transaction ID',
  `memo` text DEFAULT NULL COMMENT 'transaction description (from bank)',
  `xid` bigint(20) NOT NULL DEFAULT 0 COMMENT 'id of related tx_hdrs record',
  PRIMARY KEY (`id`),
  KEY `completed` (`completed`)
) ENGINE=InnoDB AUTO_INCREMENT=145 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Record of transfers to or from a bank account';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `r_user_industries`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `r_user_industries` (
  `id` int(11) NOT NULL AUTO_INCREMENT COMMENT 'user industry record id',
  `iid` int(11) NOT NULL DEFAULT 0 COMMENT 'industry id',
  `uid` bigint(20) NOT NULL DEFAULT 0 COMMENT 'industry id',
  PRIMARY KEY (`id`),
  KEY `uid` (`uid`),
  KEY `iid` (`iid`)
) ENGINE=InnoDB AUTO_INCREMENT=367 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='industries for each company';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `r_votes`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `r_votes` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'vote record id',
  `ballot` bigint(20) NOT NULL DEFAULT 0 COMMENT 'ballot on which this option is being graded',
  `option` bigint(20) NOT NULL DEFAULT 0 COMMENT 'option being graded',
  `grade` int(11) NOT NULL DEFAULT -1 COMMENT 'grade given by a particular voter for this option',
  `gradeMax` int(11) NOT NULL DEFAULT -1 COMMENT 'maximum grade given by a particular voter for this range-type option',
  `displayOrder` int(11) NOT NULL DEFAULT 0 COMMENT 'what order options were shown in to this voter',
  `text` longtext DEFAULT NULL COMMENT 'what was voter''s comment or moral objection to this option',
  `isVeto` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'this is a veto (not a canceled veto or mere comment)',
  `modified` int(11) NOT NULL DEFAULT 0 COMMENT 'date/time last modified',
  PRIMARY KEY (`id`),
  KEY `ballot` (`ballot`),
  KEY `option` (`option`)
) ENGINE=InnoDB AUTO_INCREMENT=16989 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='What grade a particular voter gave a particular options...';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `registry`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `registry` (
  `name` varchar(255) NOT NULL DEFAULT '' COMMENT 'The name of the function, class, or interface.',
  `type` varchar(9) NOT NULL DEFAULT '' COMMENT 'Either function or class or interface.',
  `filename` varchar(255) NOT NULL COMMENT 'Name of the file.',
  `module` varchar(255) NOT NULL DEFAULT '' COMMENT 'Name of the module the file belongs to.',
  `weight` int(11) NOT NULL DEFAULT 0 COMMENT 'The order in which this module’s hooks should be invoked relative to other modules. Equal-weighted modules are ordered by name.',
  PRIMARY KEY (`name`,`type`),
  KEY `hook` (`type`,`weight`,`module`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Each record is a function, class, or interface name and...';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `registry_file`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `registry_file` (
  `filename` varchar(255) NOT NULL COMMENT 'Path to the file.',
  `hash` varchar(64) NOT NULL COMMENT 'sha-256 hash of the file’s contents when last parsed.',
  PRIMARY KEY (`filename`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Files parsed to build the registry.';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `reset_codes`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `reset_codes` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `uid` bigint(20) NOT NULL COMMENT 'record ID of the account being reset',
  `codeHash` varchar(255) NOT NULL COMMENT 'hash of the code (never the code itself)',
  `expires` bigint(20) NOT NULL COMMENT 'date/time the code stops working',
  `tries` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'wrong guesses so far',
  `channel` enum('email','sms') NOT NULL COMMENT 'how the code was delivered',
  `created` bigint(20) NOT NULL COMMENT 'date/time the code was requested',
  `usedAt` bigint(20) DEFAULT NULL COMMENT 'date/time the code was used (empty until then)',
  PRIMARY KEY (`id`),
  KEY `uid` (`uid`,`created`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_general_ci COMMENT='one-time password reset codes, owned by the node app (cgpay)';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `semaphore`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `semaphore` (
  `name` varchar(255) NOT NULL DEFAULT '' COMMENT 'Primary Key: Unique name.',
  `value` varchar(255) NOT NULL DEFAULT '' COMMENT 'A value for the semaphore.',
  `expire` double NOT NULL COMMENT 'A Unix timestamp with microseconds indicating when the semaphore should expire.',
  PRIMARY KEY (`name`),
  KEY `value` (`value`),
  KEY `expire` (`expire`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Table for holding semaphores, locks, flags, etc. that...';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `sessions`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `sessions` (
  `uid` bigint(20) DEFAULT NULL COMMENT 'users record id',
  `acct` bigint(20) DEFAULT NULL COMMENT 'currently viewing/managing this account ID',
  `sid` varchar(128) NOT NULL COMMENT 'A session ID. The value is generated by Drupal’s session handlers.',
  `ssid` varchar(128) NOT NULL DEFAULT '' COMMENT 'Secure session ID. The value is generated by Drupal’s session handlers.',
  `hostname` varchar(128) NOT NULL DEFAULT '' COMMENT 'The IP address that last used this session ID (sid).',
  `timestamp` int(11) NOT NULL DEFAULT 0 COMMENT 'The Unix timestamp when this session last requested a page. Old records are purged by PHP automatically.',
  `cache` int(11) NOT NULL DEFAULT 0 COMMENT 'The time of this user’s last post. This is used when the site has specified a minimum_cache_lifetime. See cache_get().',
  `session` longblob DEFAULT NULL COMMENT 'The serialized contents of $_SESSION, an array of name/value pairs that persists across page requests by this session ID. Drupal loads $_SESSION from here at the start of each request and saves it at the end.',
  PRIMARY KEY (`sid`,`ssid`),
  KEY `timestamp` (`timestamp`),
  KEY `uid` (`uid`),
  KEY `ssid` (`ssid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Drupal’s session handlers read and write into the...';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `system`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `system` (
  `filename` varchar(255) NOT NULL DEFAULT '' COMMENT 'The path of the primary file for this item, relative to the Drupal root; e.g. modules/node/node.module.',
  `name` varchar(255) NOT NULL DEFAULT '' COMMENT 'The name of the item; e.g. node.',
  `type` varchar(12) NOT NULL DEFAULT '' COMMENT 'The type of the item, either module, theme, or theme_engine.',
  `owner` varchar(255) NOT NULL DEFAULT '' COMMENT 'A theme’s ’parent’ . Can be either a theme or an engine.',
  `status` int(11) NOT NULL DEFAULT 0 COMMENT 'Boolean indicating whether or not this item is enabled.',
  `bootstrap` int(11) NOT NULL DEFAULT 0 COMMENT 'Boolean indicating whether this module is loaded during Drupal’s early bootstrapping phase (e.g. even before the page cache is consulted).',
  `schema_version` smallint(6) NOT NULL DEFAULT -1 COMMENT 'The module’s database schema version number. -1 if the module is not installed (its tables do not exist); 0 or the largest N of the module’s hook_update_N() function that has either been run or existed when the module was first installed.',
  `weight` int(11) NOT NULL DEFAULT 0 COMMENT 'The order in which this module’s hooks should be invoked relative to other modules. Equal-weighted modules are ordered by name.',
  `info` blob DEFAULT NULL COMMENT 'A serialized array containing information from the module’s .info file; keys can include name, description, package, version, core, dependencies, and php.',
  PRIMARY KEY (`filename`),
  KEY `system_list` (`status`,`bootstrap`,`type`,`weight`,`name`),
  KEY `type_name` (`type`,`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='A list of all modules, themes, and theme engines that are...';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `test`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `test` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `test` varchar(255) DEFAULT NULL COMMENT 'name of the current test',
  `type` varchar(255) NOT NULL COMMENT 'type of data stored here',
  `value` longblob DEFAULT NULL COMMENT 'the data',
  PRIMARY KEY (`id`),
  KEY `test` (`test`),
  KEY `type` (`type`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='transient data while testing offline';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `tx_bads`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `tx_bads` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'record ID',
  `version` mediumint(8) NOT NULL COMMENT 'app version number',
  `deviceId` mediumtext DEFAULT NULL COMMENT 'ID of the device submitting the transaction',
  `actorId` varchar(255) DEFAULT NULL COMMENT 'account ID of the transaction initiator',
  `otherId` varchar(255) DEFAULT NULL COMMENT 'other account ID',
  `amount` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'amount to pay or charge',
  `description` mediumtext DEFAULT NULL COMMENT 'description of transactions',
  `created` bigint(20) DEFAULT NULL COMMENT 'date/time of transaction',
  `proof` varchar(255) DEFAULT NULL COMMENT 'various parameters hashed together with cardCode',
  `offline` tinyint(4) NOT NULL COMMENT 'transaction taken online (0) or offline (1)',
  `problem` mediumtext DEFAULT NULL COMMENT 'problem description',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_general_ci COMMENT='transactions that cannot be completed, requested by our app';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `tx_cats`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `tx_cats` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `category` varchar(255) DEFAULT NULL COMMENT 'category',
  `description` longtext DEFAULT NULL COMMENT 'description of category',
  `externalId` bigint(20) DEFAULT NULL COMMENT 'account record ID in external accounting program',
  `show` tinyint(4) NOT NULL DEFAULT 1 COMMENT 'whether to show this category in dropdowns',
  `line990` varchar(255) DEFAULT NULL COMMENT 'section, part, and line number of this category on IRS Form 990',
  `nick` varchar(255) DEFAULT NULL COMMENT 'nickname for the account, used to set transaction category',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=14901 DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_general_ci COMMENT='income and expense categories';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `tx_credits`;
/*!50001 DROP VIEW IF EXISTS `tx_credits`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8mb4;
/*!50001 CREATE VIEW `tx_credits` AS SELECT
 NULL AS `id`,
 NULL AS `created`,
 NULL AS `fromUid`,
 NULL AS `toUid`,
 NULL AS `amount`,
 NULL AS `xid`,
 NULL AS `purpose` */;
SET character_set_client = @saved_cs_client;
DROP TABLE IF EXISTS `tx_disputes`;
/*!50001 DROP VIEW IF EXISTS `tx_disputes`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8mb4;
/*!50001 CREATE VIEW `tx_disputes` AS SELECT
 NULL AS `id`,
 NULL AS `xid`,
 NULL AS `uid`,
 NULL AS `agentUid`,
 NULL AS `reason`,
 NULL AS `status`,
 NULL AS `deleted` */;
SET character_set_client = @saved_cs_client;
DROP TABLE IF EXISTS `tx_disputes_all`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `tx_disputes_all` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `xid` bigint(20) NOT NULL COMMENT 'id of the transaction in dispute',
  `uid` bigint(20) NOT NULL COMMENT 'id of the user who disputes the transaction',
  `agentUid` bigint(20) NOT NULL COMMENT 'id of the user who actually acted for the nominal user',
  `reason` varchar(255) NOT NULL COMMENT 'reason the transaction is being disputes',
  `status` tinyint(4) NOT NULL DEFAULT 1 COMMENT 'status of the dispute',
  `deleted` bigint(20) DEFAULT NULL COMMENT 'unix timestamp of when the dispute record was deleted, null if it hasn''t been',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='record of dispute of transaction';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `tx_entries`;
/*!50001 DROP VIEW IF EXISTS `tx_entries`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8mb4;
/*!50001 CREATE VIEW `tx_entries` AS SELECT
 NULL AS `id`,
 NULL AS `xid`,
 NULL AS `entryType`,
 NULL AS `amount`,
 NULL AS `uid`,
 NULL AS `agentUid`,
 NULL AS `description`,
 NULL AS `cat`,
 NULL AS `relType`,
 NULL AS `relatedId`,
 NULL AS `rule` */;
SET character_set_client = @saved_cs_client;
DROP TABLE IF EXISTS `tx_entries_all`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `tx_entries_all` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `xid` bigint(20) NOT NULL DEFAULT 0 COMMENT 'the ID of the transaction to which this entry belongs',
  `entryType` tinyint(4) DEFAULT NULL COMMENT 'entry type',
  `amount` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'amount, may be negative',
  `uid` bigint(20) NOT NULL COMMENT 'user id of the account to which this entry applies',
  `agentUid` bigint(20) DEFAULT NULL COMMENT 'user id of account''s agent (who approved this transaction for this account)',
  `description` mediumtext NOT NULL DEFAULT 'NULL' COMMENT 'description for this entry',
  `cat` bigint(20) DEFAULT NULL COMMENT 'related tx_cats record ID',
  `relType` varchar(1) DEFAULT NULL COMMENT 'type of related record, ''D'' for coupated, ''I'' for invoice',
  `relatedId` bigint(20) DEFAULT NULL COMMENT 'id of related record',
  `deleted` bigint(20) DEFAULT NULL COMMENT 'UNIXTIME when record was deleted, else null',
  `rule` int(11) DEFAULT NULL COMMENT 'Auxiliary transaction to which this entry is related.',
  PRIMARY KEY (`id`),
  KEY `xid` (`xid`),
  KEY `uid` (`uid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Record of a transaction line entry';
/*!40101 SET character_set_client = @saved_cs_client */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'NO_AUTO_VALUE_ON_ZERO' */ ;
DELIMITER ;;
/*!50003 CREATE*/ /*!50017*/ /*!50003 TRIGGER `insEntry` AFTER INSERT ON `tx_entries_all` FOR EACH ROW UPDATE users u SET balance=balance+NEW.amount
    WHERE NEW.uid IN (u.uid, u.jid) 
*/;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'NO_AUTO_VALUE_ON_ZERO' */ ;
DELIMITER ;;
/*!50003 CREATE*/ /*!50017*/ /*!50003 TRIGGER `updEntry` AFTER UPDATE ON `tx_entries_all` FOR EACH ROW UPDATE users u SET balance=balance
      -IF(OLD.uid IN (u.uid, u.jid), IF(OLD.deleted,0,OLD.amount), 0)
      +IF(NEW.uid IN (u.uid, u.jid), IF(NEW.deleted,0,NEW.amount), 0)
    WHERE OLD.uid IN (u.uid, u.jid) OR NEW.uid IN (u.uid, u.jid) 
*/;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'NO_AUTO_VALUE_ON_ZERO' */ ;
DELIMITER ;;
/*!50003 CREATE*/ /*!50017*/ /*!50003 TRIGGER `delEntry` AFTER DELETE ON `tx_entries_all` FOR EACH ROW UPDATE users u SET balance=balance-OLD.amount
    WHERE OLD.uid IN (u.uid, u.jid) 
*/;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
DROP TABLE IF EXISTS `tx_entries_o`;
/*!50001 DROP VIEW IF EXISTS `tx_entries_o`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8mb4;
/*!50001 CREATE VIEW `tx_entries_o` AS SELECT
 NULL AS `id`,
 NULL AS `xid`,
 NULL AS `entryType`,
 NULL AS `amount`,
 NULL AS `uid`,
 NULL AS `agentUid`,
 NULL AS `description`,
 NULL AS `cat`,
 NULL AS `relType`,
 NULL AS `relatedId`,
 NULL AS `rule` */;
SET character_set_client = @saved_cs_client;
DROP TABLE IF EXISTS `tx_hdrs`;
/*!50001 DROP VIEW IF EXISTS `tx_hdrs`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8mb4;
/*!50001 CREATE VIEW `tx_hdrs` AS SELECT
 NULL AS `xid`,
 NULL AS `actorId`,
 NULL AS `actorAgentId`,
 NULL AS `flags`,
 NULL AS `channel`,
 NULL AS `boxId`,
 NULL AS `goods`,
 NULL AS `risk`,
 NULL AS `risks`,
 NULL AS `recursId`,
 NULL AS `reversesXid`,
 NULL AS `created` */;
SET character_set_client = @saved_cs_client;
DROP TABLE IF EXISTS `tx_hdrs_all`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `tx_hdrs_all` (
  `xid` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'the unique transaction ID',
  `actorId` bigint(20) NOT NULL COMMENT 'user id of the transaction''s initiator',
  `actorAgentId` bigint(20) DEFAULT NULL COMMENT 'user id of the agent for the initiator (who actually initiated this transaction)',
  `flags` bigint(20) unsigned NOT NULL DEFAULT 0 COMMENT 'boolean characteristics and state flags',
  `channel` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'through what medium was the transaction entered',
  `boxId` bigint(20) DEFAULT NULL COMMENT 'on what machine was the transaction entered',
  `goods` tinyint(4) NOT NULL COMMENT 'kind of thing being dealt in',
  `risk` float DEFAULT NULL COMMENT 'suspiciousness rating',
  `risks` bigint(20) unsigned NOT NULL DEFAULT 0 COMMENT 'list of risk factors',
  `recursId` bigint(20) DEFAULT NULL COMMENT 'related record ID in tx_timed, for recurring or delayed transaction',
  `reversesXid` bigint(20) DEFAULT NULL COMMENT 'xid of the transaction this one reverses (if any)',
  `created` bigint(20) NOT NULL DEFAULT 0 COMMENT 'Unixtime transaction was created',
  `deleted` bigint(20) DEFAULT NULL COMMENT 'Unixtime transaction was deleted, null if it has not been',
  PRIMARY KEY (`xid`),
  UNIQUE KEY `xid` (`xid`),
  UNIQUE KEY `reversesXid` (`reversesXid`),
  KEY `actorId` (`actorId`),
  KEY `created` (`created`),
  KEY `recursId` (`recursId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Record of all rCredits transactions in the region';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `tx_requests`;
/*!50001 DROP VIEW IF EXISTS `tx_requests`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8mb4;
/*!50001 CREATE VIEW `tx_requests` AS SELECT
 NULL AS `nvid`,
 NULL AS `status`,
 NULL AS `amount`,
 NULL AS `payer`,
 NULL AS `payee`,
 NULL AS `goods`,
 NULL AS `purpose`,
 NULL AS `cat`,
 NULL AS `flags`,
 NULL AS `data`,
 NULL AS `reversesXid`,
 NULL AS `recursId`,
 NULL AS `created`,
 NULL AS `deleted` */;
SET character_set_client = @saved_cs_client;
DROP TABLE IF EXISTS `tx_requests_all`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `tx_requests_all` (
  `nvid` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'the unique invoice ID',
  `status` int(11) NOT NULL DEFAULT -1 COMMENT 'transaction record ID or status (approved, pending, denied, or paid)',
  `amount` decimal(11,2) DEFAULT NULL COMMENT 'amount to charge (negative for store credit)',
  `payer` bigint(20) DEFAULT NULL COMMENT 'user id of the payer',
  `payee` bigint(20) DEFAULT NULL COMMENT 'user id of the payee',
  `goods` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'is this an invoice for real goods and services?',
  `purpose` longtext DEFAULT NULL COMMENT 'payee''s description',
  `cat` bigint(20) DEFAULT NULL COMMENT 'related tx_cats record ID',
  `flags` bigint(20) unsigned NOT NULL DEFAULT 0 COMMENT 'boolean characteristics and state flags',
  `data` longtext DEFAULT NULL COMMENT 'miscellaneous non-searchable data (serialized array)',
  `reversesXid` bigint(20) DEFAULT NULL COMMENT 'xid of the transaction this invoice reverses (if any)',
  `recursId` bigint(20) DEFAULT NULL COMMENT 'related record ID in tx_timed, for recurring or delayed charge (or reversed payment)',
  `created` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime invoice was created',
  `deleted` bigint(20) DEFAULT NULL COMMENT 'record is deleted',
  PRIMARY KEY (`nvid`),
  KEY `payer` (`payer`),
  KEY `payee` (`payee`),
  KEY `created` (`created`),
  KEY `status` (`status`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Record of all rCredits invoices in the region';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `tx_rules`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `tx_rules` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `action` enum('pay','charge','surtx','redeem') NOT NULL COMMENT 'Action that triggers templates of this type',
  `from` bigint(20) NOT NULL COMMENT 'Who to transfer money from',
  `to` bigint(20) NOT NULL COMMENT 'Who to transfer money to',
  `amount` decimal(11,2) unsigned NOT NULL DEFAULT 0.00 COMMENT 'Fixed amount to transfer',
  `portion` decimal(7,6) unsigned NOT NULL DEFAULT 0.000000 COMMENT 'Proportional amount, e.g., 5%, to transfer, expressed as a decimal, e.g., 0.05',
  `purpose` varchar(255) NOT NULL DEFAULT '' COMMENT 'Text to appear on statements explaining this',
  `payerType` enum('anybody','account','anyCo','industry','group','person') NOT NULL COMMENT 'Type of payer',
  `payer` bigint(20) DEFAULT NULL COMMENT 'Payer party to base transaction, null if anybody',
  `payeeType` enum('anybody','account','anyCo','industry','group','person') NOT NULL COMMENT 'Type of payee',
  `payee` bigint(20) DEFAULT NULL COMMENT 'Payee party to base transaction, null if anybody',
  `minimum` decimal(11,2) unsigned NOT NULL DEFAULT 0.00 COMMENT 'Minimum amount of transaction that this template applies to',
  `useMax` int(11) unsigned DEFAULT NULL COMMENT 'Maximum number of uses per member, NULL if no max',
  `amtMax` decimal(11,2) unsigned DEFAULT NULL COMMENT 'Maximum amount to transfer per rule (if portion=0) or per transaction (if portion<>0), NULL if no limit',
  `start` bigint(20) NOT NULL COMMENT 'Start of period for which this occurrence applies',
  `end` bigint(20) DEFAULT NULL COMMENT 'End of period for which this occurrence applies, NULL if it does not end',
  `code` int(11) DEFAULT NULL COMMENT 'For gift cards the individual code',
  `template` int(11) DEFAULT NULL COMMENT 'Template of which this is an occurrence',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Occurrences of a rule';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `tx_timed`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `tx_timed` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `action` enum('pay','charge','surtx','redeem') NOT NULL COMMENT 'Action that triggers templates of this type',
  `from` bigint(20) NOT NULL COMMENT 'Who to transfer money from',
  `to` bigint(20) NOT NULL COMMENT 'Who to transfer money to',
  `amount` decimal(11,2) unsigned DEFAULT 0.00 COMMENT 'Fixed amount to transfer (NULL for sweep)',
  `portion` decimal(7,6) unsigned NOT NULL DEFAULT 0.000000 COMMENT 'Proportional amount, e.g., 5%, to transfer, expressed as a decimal, e.g., 0.05',
  `purpose` varchar(255) NOT NULL DEFAULT '' COMMENT 'Text to appear on statements explaining this',
  `payerType` enum('anybody','account','anyCo','industry','group','person') NOT NULL COMMENT 'Type of payer',
  `payer` bigint(20) DEFAULT NULL COMMENT 'Payer party to base transaction, null if anybody',
  `payeeType` enum('anybody','account','anyCo','industry','group','person') NOT NULL COMMENT 'Type of payee',
  `payee` bigint(20) DEFAULT NULL COMMENT 'Payee party to base transaction, null if anybody',
  `minimum` decimal(11,2) unsigned NOT NULL DEFAULT 0.00 COMMENT 'Minimum amount of transaction that this template applies to',
  `useMax` int(11) unsigned DEFAULT NULL COMMENT 'Maximum number of uses per member, NULL if no max',
  `amtMax` decimal(11,2) unsigned DEFAULT NULL COMMENT 'Maximum amount to transfer per rule (if portion=0) or per transaction (if portion<>0), NULL if no limit',
  `flags` bigint(20) NOT NULL DEFAULT 0 COMMENT 'transaction flag bits',
  `start` bigint(20) NOT NULL COMMENT 'Start date of first occurrence of this template',
  `end` bigint(20) DEFAULT NULL COMMENT 'Date after which no more occurrences will be created (NULL if no end)',
  `period` enum('once','day','week','month','quarter','year','forever') NOT NULL COMMENT 'The units for the period',
  `periods` int(11) unsigned NOT NULL DEFAULT 1 COMMENT 'Every how many periods a rule will be generated',
  `duration` enum('once','day','week','month','quarter','year','forever') NOT NULL COMMENT 'The unit of duration',
  `durations` int(11) unsigned NOT NULL DEFAULT 1 COMMENT 'How many duration units an occurrence is valid for',
  `stripeId` varchar(255) DEFAULT NULL COMMENT 'setupIntentId from Stripe',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Templates for auxiliary transactions';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `txs`;
/*!50001 DROP VIEW IF EXISTS `txs`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8mb4;
/*!50001 CREATE VIEW `txs` AS SELECT
 NULL AS `xid`,
 NULL AS `actorId`,
 NULL AS `actorAgentId`,
 NULL AS `flags`,
 NULL AS `channel`,
 NULL AS `boxId`,
 NULL AS `goods`,
 NULL AS `risk`,
 NULL AS `risks`,
 NULL AS `recursId`,
 NULL AS `reversesXid`,
 NULL AS `created`,
 NULL AS `type`,
 NULL AS `amt`,
 NULL AS `rule`,
 NULL AS `relType`,
 NULL AS `rel`,
 NULL AS `eid`,
 NULL AS `for2`,
 NULL AS `uid2`,
 NULL AS `agt2`,
 NULL AS `cat2`,
 NULL AS `for1`,
 NULL AS `uid1`,
 NULL AS `agt1`,
 NULL AS `cat1` */;
SET character_set_client = @saved_cs_client;
DROP TABLE IF EXISTS `txs2`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `txs2` (
  `txid` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'the unique transaction ID',
  `amount` decimal(11,2) DEFAULT NULL COMMENT 'amount of transfer',
  `payee` bigint(20) DEFAULT NULL COMMENT 'CG account record ID',
  `created` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime transaction was created',
  `completed` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime transaction was completed',
  `deposit` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime transfer check was printed and deposited',
  `bankAccount` blob DEFAULT NULL COMMENT 'Bank account for the transfer',
  `isSavings` tinyint(4) DEFAULT NULL COMMENT '1 if bankAccount is a savings account',
  `risk` float DEFAULT NULL COMMENT 'suspiciousness rating',
  `risks` bigint(20) unsigned NOT NULL DEFAULT 0 COMMENT 'list of risk factors',
  `bankTxId` bigint(20) DEFAULT NULL COMMENT 'bank transaction ID',
  `channel` tinyint(4) DEFAULT NULL COMMENT 'through what medium was the transaction entered',
  `xid` bigint(20) NOT NULL DEFAULT 0 COMMENT 'id of related tx_hdrs record',
  `pid` bigint(20) DEFAULT NULL COMMENT 'related people record ID',
  PRIMARY KEY (`txid`),
  KEY `created` (`created`),
  KEY `pid` (`pid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Record of bank transfers in the region';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `txs2_bank`;
/*!50001 DROP VIEW IF EXISTS `txs2_bank`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8mb4;
/*!50001 CREATE VIEW `txs2_bank` AS SELECT
 NULL AS `txid`,
 NULL AS `amount`,
 NULL AS `payee`,
 NULL AS `created`,
 NULL AS `completed`,
 NULL AS `deposit`,
 NULL AS `bankAccount`,
 NULL AS `isSavings`,
 NULL AS `risk`,
 NULL AS `risks`,
 NULL AS `bankTxId`,
 NULL AS `channel`,
 NULL AS `xid`,
 NULL AS `pid` */;
SET character_set_client = @saved_cs_client;
DROP TABLE IF EXISTS `txs2_outer`;
/*!50001 DROP VIEW IF EXISTS `txs2_outer`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8mb4;
/*!50001 CREATE VIEW `txs2_outer` AS SELECT
 NULL AS `txid`,
 NULL AS `amount`,
 NULL AS `payee`,
 NULL AS `created`,
 NULL AS `completed`,
 NULL AS `deposit`,
 NULL AS `bankAccount`,
 NULL AS `isSavings`,
 NULL AS `risk`,
 NULL AS `risks`,
 NULL AS `bankTxId`,
 NULL AS `channel`,
 NULL AS `xid`,
 NULL AS `pid` */;
SET character_set_client = @saved_cs_client;
DROP TABLE IF EXISTS `txs_all`;
/*!50001 DROP VIEW IF EXISTS `txs_all`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8mb4;
/*!50001 CREATE VIEW `txs_all` AS SELECT
 NULL AS `xid`,
 NULL AS `actorId`,
 NULL AS `actorAgentId`,
 NULL AS `flags`,
 NULL AS `channel`,
 NULL AS `boxId`,
 NULL AS `goods`,
 NULL AS `risk`,
 NULL AS `risks`,
 NULL AS `recursId`,
 NULL AS `reversesXid`,
 NULL AS `created`,
 NULL AS `type`,
 NULL AS `amt`,
 NULL AS `rule`,
 NULL AS `relType`,
 NULL AS `rel`,
 NULL AS `eid`,
 NULL AS `for2`,
 NULL AS `uid2`,
 NULL AS `agt2`,
 NULL AS `cat2`,
 NULL AS `for1`,
 NULL AS `uid1`,
 NULL AS `agt1`,
 NULL AS `cat1` */;
SET character_set_client = @saved_cs_client;
DROP TABLE IF EXISTS `txs_aux`;
/*!50001 DROP VIEW IF EXISTS `txs_aux`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8mb4;
/*!50001 CREATE VIEW `txs_aux` AS SELECT
 NULL AS `xid`,
 NULL AS `actorId`,
 NULL AS `actorAgentId`,
 NULL AS `flags`,
 NULL AS `channel`,
 NULL AS `boxId`,
 NULL AS `goods`,
 NULL AS `risk`,
 NULL AS `risks`,
 NULL AS `recursId`,
 NULL AS `reversesXid`,
 NULL AS `created`,
 NULL AS `type`,
 NULL AS `amt`,
 NULL AS `rule`,
 NULL AS `relType`,
 NULL AS `rel`,
 NULL AS `eid`,
 NULL AS `for2`,
 NULL AS `uid2`,
 NULL AS `agt2`,
 NULL AS `cat2`,
 NULL AS `for1`,
 NULL AS `uid1`,
 NULL AS `agt1`,
 NULL AS `cat1` */;
SET character_set_client = @saved_cs_client;
DROP TABLE IF EXISTS `txs_bank`;
/*!50001 DROP VIEW IF EXISTS `txs_bank`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8mb4;
/*!50001 CREATE VIEW `txs_bank` AS SELECT
 NULL AS `xid`,
 NULL AS `actorId`,
 NULL AS `actorAgentId`,
 NULL AS `flags`,
 NULL AS `channel`,
 NULL AS `boxId`,
 NULL AS `goods`,
 NULL AS `risk`,
 NULL AS `risks`,
 NULL AS `recursId`,
 NULL AS `reversesXid`,
 NULL AS `created`,
 NULL AS `type`,
 NULL AS `amt`,
 NULL AS `rule`,
 NULL AS `relType`,
 NULL AS `rel`,
 NULL AS `eid`,
 NULL AS `for2`,
 NULL AS `uid2`,
 NULL AS `agt2`,
 NULL AS `cat2`,
 NULL AS `for1`,
 NULL AS `uid1`,
 NULL AS `agt1`,
 NULL AS `cat1` */;
SET character_set_client = @saved_cs_client;
DROP TABLE IF EXISTS `txs_bank_only`;
/*!50001 DROP VIEW IF EXISTS `txs_bank_only`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8mb4;
/*!50001 CREATE VIEW `txs_bank_only` AS SELECT
 NULL AS `xid`,
 NULL AS `actorId`,
 NULL AS `actorAgentId`,
 NULL AS `flags`,
 NULL AS `channel`,
 NULL AS `boxId`,
 NULL AS `goods`,
 NULL AS `risk`,
 NULL AS `risks`,
 NULL AS `recursId`,
 NULL AS `reversesXid`,
 NULL AS `created`,
 NULL AS `type`,
 NULL AS `amt`,
 NULL AS `rule`,
 NULL AS `relType`,
 NULL AS `rel`,
 NULL AS `eid`,
 NULL AS `for2`,
 NULL AS `uid2`,
 NULL AS `agt2`,
 NULL AS `cat2`,
 NULL AS `for1`,
 NULL AS `uid1`,
 NULL AS `agt1`,
 NULL AS `cat1` */;
SET character_set_client = @saved_cs_client;
DROP TABLE IF EXISTS `txs_deleted`;
/*!50001 DROP VIEW IF EXISTS `txs_deleted`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8mb4;
/*!50001 CREATE VIEW `txs_deleted` AS SELECT
 NULL AS `deleted`,
 NULL AS `xid`,
 NULL AS `actorId`,
 NULL AS `actorAgentId`,
 NULL AS `flags`,
 NULL AS `channel`,
 NULL AS `boxId`,
 NULL AS `goods`,
 NULL AS `risk`,
 NULL AS `risks`,
 NULL AS `recursId`,
 NULL AS `reversesXid`,
 NULL AS `created`,
 NULL AS `type`,
 NULL AS `amt`,
 NULL AS `rule`,
 NULL AS `relType`,
 NULL AS `rel`,
 NULL AS `eid`,
 NULL AS `for2`,
 NULL AS `uid2`,
 NULL AS `agt2`,
 NULL AS `cat2`,
 NULL AS `for1`,
 NULL AS `uid1`,
 NULL AS `agt1`,
 NULL AS `cat1` */;
SET character_set_client = @saved_cs_client;
DROP TABLE IF EXISTS `txs_ever`;
/*!50001 DROP VIEW IF EXISTS `txs_ever`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8mb4;
/*!50001 CREATE VIEW `txs_ever` AS SELECT
 NULL AS `deleted`,
 NULL AS `xid`,
 NULL AS `actorId`,
 NULL AS `actorAgentId`,
 NULL AS `flags`,
 NULL AS `channel`,
 NULL AS `boxId`,
 NULL AS `goods`,
 NULL AS `risk`,
 NULL AS `risks`,
 NULL AS `recursId`,
 NULL AS `reversesXid`,
 NULL AS `created`,
 NULL AS `type`,
 NULL AS `amt`,
 NULL AS `rule`,
 NULL AS `relType`,
 NULL AS `rel`,
 NULL AS `eid`,
 NULL AS `for2`,
 NULL AS `uid2`,
 NULL AS `agt2`,
 NULL AS `cat2`,
 NULL AS `for1`,
 NULL AS `uid1`,
 NULL AS `agt1`,
 NULL AS `cat1` */;
SET character_set_client = @saved_cs_client;
DROP TABLE IF EXISTS `txs_noreverse`;
/*!50001 DROP VIEW IF EXISTS `txs_noreverse`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8mb4;
/*!50001 CREATE VIEW `txs_noreverse` AS SELECT
 NULL AS `xid`,
 NULL AS `actorId`,
 NULL AS `actorAgentId`,
 NULL AS `flags`,
 NULL AS `channel`,
 NULL AS `boxId`,
 NULL AS `goods`,
 NULL AS `risk`,
 NULL AS `risks`,
 NULL AS `recursId`,
 NULL AS `reversesXid`,
 NULL AS `created`,
 NULL AS `type`,
 NULL AS `amt`,
 NULL AS `rule`,
 NULL AS `relType`,
 NULL AS `rel`,
 NULL AS `eid`,
 NULL AS `for2`,
 NULL AS `uid2`,
 NULL AS `agt2`,
 NULL AS `cat2`,
 NULL AS `for1`,
 NULL AS `uid1`,
 NULL AS `agt1`,
 NULL AS `cat1` */;
SET character_set_client = @saved_cs_client;
DROP TABLE IF EXISTS `txs_outer`;
/*!50001 DROP VIEW IF EXISTS `txs_outer`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8mb4;
/*!50001 CREATE VIEW `txs_outer` AS SELECT
 NULL AS `xid`,
 NULL AS `actorId`,
 NULL AS `actorAgentId`,
 NULL AS `flags`,
 NULL AS `channel`,
 NULL AS `boxId`,
 NULL AS `goods`,
 NULL AS `risk`,
 NULL AS `risks`,
 NULL AS `recursId`,
 NULL AS `reversesXid`,
 NULL AS `created`,
 NULL AS `type`,
 NULL AS `amt`,
 NULL AS `rule`,
 NULL AS `relType`,
 NULL AS `rel`,
 NULL AS `eid`,
 NULL AS `for2`,
 NULL AS `uid2`,
 NULL AS `agt2`,
 NULL AS `cat2`,
 NULL AS `for1`,
 NULL AS `uid1`,
 NULL AS `agt1`,
 NULL AS `cat1` */;
SET character_set_client = @saved_cs_client;
DROP TABLE IF EXISTS `txs_prime`;
/*!50001 DROP VIEW IF EXISTS `txs_prime`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8mb4;
/*!50001 CREATE VIEW `txs_prime` AS SELECT
 NULL AS `xid`,
 NULL AS `actorId`,
 NULL AS `actorAgentId`,
 NULL AS `flags`,
 NULL AS `channel`,
 NULL AS `boxId`,
 NULL AS `goods`,
 NULL AS `risk`,
 NULL AS `risks`,
 NULL AS `recursId`,
 NULL AS `reversesXid`,
 NULL AS `created`,
 NULL AS `type`,
 NULL AS `amt`,
 NULL AS `rule`,
 NULL AS `relType`,
 NULL AS `rel`,
 NULL AS `eid`,
 NULL AS `for2`,
 NULL AS `uid2`,
 NULL AS `agt2`,
 NULL AS `cat2`,
 NULL AS `for1`,
 NULL AS `uid1`,
 NULL AS `agt1`,
 NULL AS `cat1` */;
SET character_set_client = @saved_cs_client;
DROP TABLE IF EXISTS `txs_proper`;
/*!50001 DROP VIEW IF EXISTS `txs_proper`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8mb4;
/*!50001 CREATE VIEW `txs_proper` AS SELECT
 NULL AS `xid`,
 NULL AS `actorId`,
 NULL AS `actorAgentId`,
 NULL AS `flags`,
 NULL AS `channel`,
 NULL AS `boxId`,
 NULL AS `goods`,
 NULL AS `risk`,
 NULL AS `risks`,
 NULL AS `recursId`,
 NULL AS `reversesXid`,
 NULL AS `created`,
 NULL AS `type`,
 NULL AS `amt`,
 NULL AS `rule`,
 NULL AS `relType`,
 NULL AS `rel`,
 NULL AS `eid`,
 NULL AS `for2`,
 NULL AS `uid2`,
 NULL AS `agt2`,
 NULL AS `cat2`,
 NULL AS `for1`,
 NULL AS `uid1`,
 NULL AS `agt1`,
 NULL AS `cat1` */;
SET character_set_client = @saved_cs_client;
DROP TABLE IF EXISTS `txs_rebate`;
/*!50001 DROP VIEW IF EXISTS `txs_rebate`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8mb4;
/*!50001 CREATE VIEW `txs_rebate` AS SELECT
 NULL AS `xid`,
 NULL AS `actorId`,
 NULL AS `actorAgentId`,
 NULL AS `flags`,
 NULL AS `channel`,
 NULL AS `boxId`,
 NULL AS `goods`,
 NULL AS `risk`,
 NULL AS `risks`,
 NULL AS `recursId`,
 NULL AS `reversesXid`,
 NULL AS `created`,
 NULL AS `type`,
 NULL AS `amt`,
 NULL AS `rule`,
 NULL AS `relType`,
 NULL AS `rel`,
 NULL AS `eid`,
 NULL AS `for2`,
 NULL AS `uid2`,
 NULL AS `agt2`,
 NULL AS `cat2`,
 NULL AS `for1`,
 NULL AS `uid1`,
 NULL AS `agt1`,
 NULL AS `cat1` */;
SET character_set_client = @saved_cs_client;
DROP TABLE IF EXISTS `txs_usd_fee`;
/*!50001 DROP VIEW IF EXISTS `txs_usd_fee`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8mb4;
/*!50001 CREATE VIEW `txs_usd_fee` AS SELECT
 NULL AS `xid`,
 NULL AS `actorId`,
 NULL AS `actorAgentId`,
 NULL AS `flags`,
 NULL AS `channel`,
 NULL AS `boxId`,
 NULL AS `goods`,
 NULL AS `risk`,
 NULL AS `risks`,
 NULL AS `recursId`,
 NULL AS `reversesXid`,
 NULL AS `created`,
 NULL AS `type`,
 NULL AS `amt`,
 NULL AS `rule`,
 NULL AS `relType`,
 NULL AS `rel`,
 NULL AS `eid`,
 NULL AS `for2`,
 NULL AS `uid2`,
 NULL AS `agt2`,
 NULL AS `cat2`,
 NULL AS `for1`,
 NULL AS `uid1`,
 NULL AS `agt1`,
 NULL AS `cat1` */;
SET character_set_client = @saved_cs_client;
DROP TABLE IF EXISTS `u_company`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `u_company` (
  `uid` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'account record ID',
  `coType` tinyint(4) DEFAULT NULL COMMENT 'type of entity',
  `contact` varchar(255) DEFAULT NULL COMMENT 'whom to contact about this account',
  `contactTitle` tinytext DEFAULT NULL COMMENT 'contact person''s position in the organization',
  `website` text DEFAULT NULL COMMENT 'company website domain',
  `description` longtext DEFAULT NULL COMMENT 'long markdown description',
  `shortDesc` text DEFAULT NULL COMMENT 'one line description',
  `selling` longtext DEFAULT NULL COMMENT 'list of typical transaction descriptions',
  `coFlags` bigint(20) unsigned NOT NULL DEFAULT 0 COMMENT 'miscellaneous flag bits',
  `founded` bigint(20) DEFAULT NULL COMMENT 'date the company was founded',
  `gross` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'average annual gross receipts',
  `zips` text DEFAULT NULL COMMENT 'zip regex for geographic region the company serves',
  `employees` int(11) NOT NULL DEFAULT 0 COMMENT 'number of employees',
  `staleNudge` int(11) NOT NULL DEFAULT 7 COMMENT 'how many days to wait before reminding customer to pay',
  `giftThanks` longtext DEFAULT NULL COMMENT 'extra email text for donation thanks',
  `payrollStart` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime date last payroll started',
  `payrollEnd` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime date last payroll ended',
  `mission` longtext DEFAULT NULL COMMENT 'the organization''s mission',
  `activities` longtext DEFAULT NULL COMMENT 'what the (sponsored) organization actually does to advance its mission',
  `checksIn` mediumint(8) DEFAULT NULL COMMENT 'expected number of checks received monthly',
  `checksOut` mediumint(8) DEFAULT NULL COMMENT 'expected number of outgoing payments monthly',
  `logo` text DEFAULT NULL COMMENT 'company logo URL',
  `target` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'fundraising target amount',
  `targetStart` int(11) DEFAULT NULL COMMENT 'starting date of fundraising project',
  PRIMARY KEY (`uid`)
) ENGINE=InnoDB AUTO_INCREMENT=40326000000003 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Companies';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `u_focuses`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `u_focuses` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `uid` bigint(20) DEFAULT NULL COMMENT 'staff member''s account ID',
  `project` varchar(255) DEFAULT NULL COMMENT 'what they focused on',
  `percent` decimal(6,3) NOT NULL DEFAULT 100.000 COMMENT 'what percentage of their time',
  PRIMARY KEY (`id`),
  KEY `uid` (`uid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_general_ci COMMENT='how staff members spend their time';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `u_groupies`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `u_groupies` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `uid` bigint(20) NOT NULL,
  `grpId` int(11) NOT NULL,
  `isMember` tinyint(1) NOT NULL DEFAULT 0 COMMENT 'User is a member of the group',
  `canAdd` tinyint(1) NOT NULL DEFAULT 0 COMMENT 'User can add other users to the group',
  `canRemove` tinyint(1) NOT NULL DEFAULT 0 COMMENT 'User can remove other users from the group',
  `start` bigint(20) NOT NULL,
  `end` bigint(20) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `uid` (`uid`,`start`),
  KEY `grpId` (`grpId`,`start`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC;
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `u_groups`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `u_groups` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(40) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC;
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `u_photo`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `u_photo` (
  `uid` bigint(20) NOT NULL COMMENT 'account record id',
  `photo` longblob DEFAULT NULL COMMENT 'member photo',
  `thumb` blob DEFAULT NULL COMMENT 'small version of photo',
  PRIMARY KEY (`uid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='one photo for each account';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `u_relations`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `u_relations` (
  `reid` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'relationship record id',
  `main` bigint(20) DEFAULT NULL COMMENT 'uid of the account to which others are related',
  `other` bigint(20) DEFAULT NULL COMMENT 'uid of an other account related to this account',
  `otherNum` int(11) NOT NULL DEFAULT 0 COMMENT 'sequence number of the other (starts with 1)',
  `permission` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'what type of permission the other has on the main account',
  `code` varchar(50) DEFAULT NULL COMMENT 'the (main) company''s account ID for this other',
  `data` longtext DEFAULT NULL COMMENT 'serialized array of parameters',
  `flags` bigint(20) unsigned NOT NULL DEFAULT 0 COMMENT 'boolean type flags',
  `created` int(11) DEFAULT NULL COMMENT 'Unixtime record created',
  `txNotifyEmail` tinyint(1) NOT NULL DEFAULT 0 COMMENT 'this relation wants email notifications for the main account''s money-movement transactions',
  `txNotifyText` tinyint(1) NOT NULL DEFAULT 0 COMMENT 'this relation wants SMS notifications for the main account''s money-movement transactions',
  PRIMARY KEY (`reid`),
  KEY `main` (`main`),
  KEY `other` (`other`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Who can manage which accounts, and how';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `u_shout`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `u_shout` (
  `uid` bigint(20) NOT NULL COMMENT 'record ID in users table',
  `org` varchar(255) DEFAULT NULL COMMENT 'signer''s organization, if any',
  `title` varchar(255) DEFAULT NULL COMMENT 'signer''s title',
  `website` varchar(255) DEFAULT NULL COMMENT 'organization website, if any',
  `quote` longtext DEFAULT NULL COMMENT 'what benefit this signer sees for the community',
  `created` int(11) DEFAULT NULL COMMENT 'creation date',
  `usePhoto` enum('0','1') NOT NULL COMMENT 'okay to use this member''s photo in a publicity collage?',
  `postPhoto` enum('0','1') NOT NULL COMMENT 'okay to use this member''s photo in social media?',
  `sawVideo` enum('0','1') NOT NULL COMMENT 'did the member see the video?',
  `rating` int(11) NOT NULL DEFAULT 0 COMMENT 'how awesome is the quote, 0=not',
  PRIMARY KEY (`uid`),
  UNIQUE KEY `uid` (`uid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='people who have signed a public statement of support';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `u_track`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `u_track` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'record ID',
  `uid` bigint(20) DEFAULT NULL COMMENT 'related account record ID',
  `type` varchar(255) DEFAULT NULL COMMENT 'what type of email or email address (for invite)',
  `sent` varchar(11) DEFAULT NULL COMMENT 'latest date sent',
  `seen` varchar(11) DEFAULT NULL COMMENT 'latest date opened',
  PRIMARY KEY (`id`),
  KEY `uid` (`uid`),
  KEY `type` (`type`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='contact information for non-members';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `users`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `users` (
  `uid` bigint(20) NOT NULL COMMENT 'account record ID',
  `name` varchar(60) DEFAULT NULL COMMENT 'unique user name',
  `pass` varchar(128) DEFAULT NULL COMMENT 'account password',
  `email` varchar(254) DEFAULT NULL COMMENT 'email address',
  `created` int(11) NOT NULL DEFAULT 0 COMMENT 'Unix date and time record was created',
  `access` int(11) NOT NULL DEFAULT 0 COMMENT 'Unix date and time account was last accessed',
  `login` int(11) NOT NULL DEFAULT 0 COMMENT 'Unix date and time user last signed in',
  `picture` int(11) NOT NULL DEFAULT 0 COMMENT 'used for temporary storage when generating statistics',
  `data` longtext DEFAULT NULL COMMENT 'serialized associative array of miscellaneous fields (not encrypted)',
  `flags` bigint(20) unsigned NOT NULL DEFAULT 0 COMMENT 'boolean permissions and state flags',
  `jid` bigint(20) NOT NULL DEFAULT 0 COMMENT 'account ID of joined account (0 if none)',
  `steps` bigint(20) unsigned NOT NULL DEFAULT 0 COMMENT 'boolean account setup steps completed',
  `changes` longblob DEFAULT NULL COMMENT 'changes made to the account',
  `community` bigint(20) NOT NULL DEFAULT 0 COMMENT 'account ID of this account''s Common Good Community',
  `secure` mediumblob DEFAULT NULL COMMENT 'encrypted data',
  `vsecure` blob DEFAULT NULL COMMENT 'hyper-encrypted data',
  `fullName` varchar(255) DEFAULT NULL COMMENT 'full name of the individual or entity',
  `phone` varchar(255) DEFAULT NULL COMMENT 'contact phone (no country code, no punctuation)',
  `city` varchar(60) DEFAULT NULL COMMENT 'municipality',
  `state` int(5) NOT NULL DEFAULT 0 COMMENT 'state/province index',
  `zip` varchar(255) DEFAULT NULL COMMENT 'postal code for physical address (no punctuation)',
  `country` int(4) NOT NULL DEFAULT 0 COMMENT 'country index',
  `latitude` decimal(11,8) NOT NULL DEFAULT 0.00000000 COMMENT 'latitude of account''s physical address',
  `longitude` decimal(11,8) NOT NULL DEFAULT 0.00000000 COMMENT 'longitude of account''s physical address',
  `notes` longtext DEFAULT NULL COMMENT 'miscellaneous notes about the user or the account',
  `tickle` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime to tickle an admin about this account',
  `activated` int(11) NOT NULL DEFAULT 0 COMMENT 'when was the account activated',
  `helper` bigint(20) NOT NULL DEFAULT 0 COMMENT 'account that invited this person or company',
  `iCode` int(11) NOT NULL DEFAULT 0 COMMENT 'sequence number of helper invitation',
  `signed` int(11) NOT NULL DEFAULT 0 COMMENT 'when did this person sign the Common Good Agreement',
  `signedBy` varchar(60) DEFAULT NULL COMMENT 'who signed the agreement (on behalf of the account)',
  `savingsAdd` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'chosen amount to hold as savings, beyond rewards',
  `saveWeekly` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'chosen amount to increase minimum (target balance) by, weekly',
  `floor` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'negative credit line',
  `minimum` decimal(11,2) DEFAULT NULL COMMENT 'chosen target balance (for automatic refills)',
  `crumbs` decimal(6,3) NOT NULL DEFAULT 0.000 COMMENT 'percentage of each transaction to donate to CG',
  `backing` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'amount account-holder chose to back',
  `backingDate` bigint(20) NOT NULL DEFAULT 0 COMMENT 'date account-holder started backing',
  `backingNext` decimal(11,2) DEFAULT NULL COMMENT 'lower backing amount for the next year',
  `food` decimal(6,3) NOT NULL DEFAULT 0.000 COMMENT 'percentage of each food purchase to donate to the food fund',
  `balance` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'balance, not including rewards (cached)',
  `rewards` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'total incentive rewards to date (cached)',
  `committed` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'amount committed (for donations to CGF)',
  `risk` float DEFAULT NULL COMMENT 'today''s suspiciousness rating',
  `risks` bigint(20) unsigned NOT NULL DEFAULT 0 COMMENT 'list of risk factors',
  `trust` float NOT NULL DEFAULT 0 COMMENT 'how much this person is trusted by others in the community (0 for companies)',
  `stats` longtext DEFAULT NULL COMMENT 'account statistics',
  `notices` text DEFAULT NULL COMMENT 'when to send what kind of notice',
  `lastip` varchar(39) DEFAULT NULL COMMENT 'latest IP address used',
  `preid` bigint(20) NOT NULL DEFAULT 0 COMMENT 'signup record ID',
  `special` longtext DEFAULT NULL COMMENT 'special transient data',
  `source` mediumtext DEFAULT NULL COMMENT 'how did the member hear about us?',
  PRIMARY KEY (`uid`),
  KEY `access` (`access`),
  KEY `created` (`created`),
  KEY `mail` (`email`),
  KEY `picture` (`picture`),
  KEY `name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Stores user data.';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `variable`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `variable` (
  `name` varchar(128) NOT NULL DEFAULT '' COMMENT 'The name of the variable.',
  `value` longblob NOT NULL COMMENT 'The value of the variable.',
  PRIMARY KEY (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Named variable/value pairs created by Drupal core or any...';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `x_company`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `x_company` (
  `deleted` bigint(20) DEFAULT NULL COMMENT 'Unixtime record was deleted',
  `uid` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'account record ID',
  `coType` tinyint(4) DEFAULT NULL COMMENT 'type of entity',
  `contact` varchar(255) DEFAULT NULL COMMENT 'whom to contact about this account',
  `contactTitle` tinytext DEFAULT NULL COMMENT 'contact person''s position in the organization',
  `website` text DEFAULT NULL COMMENT 'company website domain',
  `description` longtext DEFAULT NULL COMMENT 'long markdown description',
  `shortDesc` text DEFAULT NULL COMMENT 'one line description',
  `selling` longtext DEFAULT NULL COMMENT 'list of typical transaction descriptions',
  `coFlags` bigint(20) unsigned NOT NULL DEFAULT 0 COMMENT 'miscellaneous flag bits',
  `founded` int(11) DEFAULT NULL COMMENT 'date the company was founded',
  `gross` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'average annual gross receipts',
  `zips` text DEFAULT NULL COMMENT 'zip regex for geographic region the company serves',
  `employees` int(11) NOT NULL DEFAULT 0 COMMENT 'number of employees',
  `staleNudge` int(11) NOT NULL DEFAULT 7 COMMENT 'how many days to wait before reminding customer to pay',
  `giftThanks` longtext DEFAULT NULL COMMENT 'extra email text for donation thanks',
  `payrollStart` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime date last payroll started',
  `payrollEnd` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime date last payroll ended',
  `mission` longtext DEFAULT NULL COMMENT 'the organization''s mission',
  `activities` longtext DEFAULT NULL COMMENT 'what the (sponsored) organization actually does to advance its mission',
  `checksIn` mediumint(8) DEFAULT NULL COMMENT 'expected number of checks received monthly',
  `checksOut` mediumint(8) DEFAULT NULL COMMENT 'expected number of outgoing payments monthly',
  `logo` text DEFAULT NULL COMMENT 'company logo URL',
  `target` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'fundraising target amount',
  `targetStart` int(11) DEFAULT NULL COMMENT 'starting date of fundraising project',
  PRIMARY KEY (`uid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Companies';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `x_photo`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `x_photo` (
  `deleted` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime record was deleted',
  `uid` bigint(20) NOT NULL COMMENT 'account record id',
  `photo` longblob DEFAULT NULL COMMENT 'member photo',
  `thumb` blob DEFAULT NULL COMMENT 'small version of photo',
  PRIMARY KEY (`uid`,`deleted`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='one photo for each account';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `x_relations`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `x_relations` (
  `deleted` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime record was deleted',
  `reid` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'relationship record id',
  `main` bigint(20) DEFAULT NULL COMMENT 'uid of the account to which others are related',
  `other` bigint(20) DEFAULT NULL COMMENT 'uid of an other account related to this account',
  `otherNum` int(11) NOT NULL DEFAULT 0 COMMENT 'sequence number of the other (starts with 1)',
  `permission` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'what type of permission the other has on the main account',
  `code` varchar(50) DEFAULT NULL COMMENT 'the (main) company''s account ID for this other',
  `data` longtext DEFAULT NULL COMMENT 'serialized array of parameters',
  `flags` bigint(20) unsigned NOT NULL DEFAULT 0 COMMENT 'boolean type flags',
  `created` int(11) DEFAULT NULL COMMENT 'Unixtime record created',
  `txNotifyEmail` tinyint(1) NOT NULL DEFAULT 0 COMMENT 'this relation wants email notifications for the main account''s money-movement transactions',
  `txNotifyText` tinyint(1) NOT NULL DEFAULT 0 COMMENT 'this relation wants SMS notifications for the main account''s money-movement transactions',
  PRIMARY KEY (`reid`,`deleted`),
  KEY `main` (`main`),
  KEY `other` (`other`)
) ENGINE=InnoDB AUTO_INCREMENT=40326000000006 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Who can manage which accounts, and how';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `x_shout`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `x_shout` (
  `deleted` bigint(20) DEFAULT NULL COMMENT 'Unixtime record was deleted',
  `uid` bigint(20) NOT NULL COMMENT 'record ID in users table',
  `org` varchar(255) DEFAULT NULL COMMENT 'signer''s organization, if any',
  `title` varchar(255) DEFAULT NULL COMMENT 'signer''s title',
  `website` varchar(255) DEFAULT NULL COMMENT 'organization website, if any',
  `quote` longtext DEFAULT NULL COMMENT 'what benefit this signer sees for the community',
  `created` int(11) DEFAULT NULL COMMENT 'creation date',
  `usePhoto` enum('0','1') NOT NULL COMMENT 'okay to use this member''s photo in a publicity collage?',
  `postPhoto` enum('0','1') NOT NULL COMMENT 'okay to use this member''s photo in social media?',
  `sawVideo` enum('0','1') NOT NULL COMMENT 'did the member see the video?',
  `rating` int(11) NOT NULL DEFAULT 0 COMMENT 'how awesome is the quote, 0=not',
  PRIMARY KEY (`uid`),
  UNIQUE KEY `uid` (`uid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='people who have signed a public statement of support';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `x_txs2`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `x_txs2` (
  `deleted` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime record was deleted',
  `txid` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'the unique transaction ID',
  `amount` decimal(11,2) DEFAULT NULL COMMENT 'amount of transfer',
  `payee` bigint(20) DEFAULT NULL COMMENT 'CG account record ID',
  `created` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime transaction was created',
  `completed` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime transaction was completed',
  `deposit` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime transfer check was printed and deposited',
  `bankAccount` blob DEFAULT NULL COMMENT 'Bank account for the transfer',
  `isSavings` tinyint(4) DEFAULT NULL COMMENT '1 if bankAccount is a savings account',
  `risk` float DEFAULT NULL COMMENT 'suspiciousness rating',
  `risks` bigint(20) unsigned NOT NULL DEFAULT 0 COMMENT 'list of risk factors',
  `bankTxId` bigint(20) DEFAULT NULL COMMENT 'bank transaction ID',
  `channel` tinyint(4) DEFAULT NULL COMMENT 'through what medium was the transaction entered',
  `xid` bigint(20) NOT NULL DEFAULT 0 COMMENT 'id of related tx_hdrs record',
  `pid` bigint(20) DEFAULT NULL COMMENT 'related people record ID',
  PRIMARY KEY (`txid`,`deleted`),
  KEY `created` (`created`),
  KEY `pid` (`pid`)
) ENGINE=InnoDB AUTO_INCREMENT=6131340 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Record of USD (Dwolla) transactions in the region';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `x_users`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `x_users` (
  `deleted` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime record was deleted',
  `uid` bigint(20) NOT NULL COMMENT 'account record ID',
  `name` varchar(60) DEFAULT NULL COMMENT 'unique user name',
  `pass` varchar(128) DEFAULT NULL COMMENT 'account password',
  `email` varchar(254) DEFAULT NULL COMMENT 'email address',
  `created` int(11) NOT NULL DEFAULT 0 COMMENT 'Unix date and time record was created',
  `access` int(11) NOT NULL DEFAULT 0 COMMENT 'Unix date and time account was last accessed',
  `login` int(11) NOT NULL DEFAULT 0 COMMENT 'Unix date and time user last signed in',
  `picture` int(11) NOT NULL DEFAULT 0 COMMENT 'used for temporary storage when generating statistics',
  `data` longtext DEFAULT NULL COMMENT 'serialized associative array of miscellaneous fields (not encrypted)',
  `flags` bigint(20) unsigned NOT NULL DEFAULT 0 COMMENT 'boolean permissions and state flags',
  `jid` bigint(20) NOT NULL DEFAULT 0 COMMENT 'account ID of joined account (0 if none)',
  `steps` bigint(20) unsigned NOT NULL DEFAULT 0 COMMENT 'boolean account setup steps completed',
  `changes` longblob DEFAULT NULL COMMENT 'changes made to the account',
  `community` bigint(20) NOT NULL DEFAULT 0 COMMENT 'account ID of this account''s Common Good Community',
  `secure` mediumblob DEFAULT NULL COMMENT 'encrypted data',
  `vsecure` blob DEFAULT NULL COMMENT 'hyper-encrypted data',
  `fullName` varchar(255) DEFAULT NULL COMMENT 'full name of the individual or entity',
  `phone` varchar(255) DEFAULT NULL COMMENT 'contact phone (no country code, no punctuation)',
  `city` varchar(60) DEFAULT NULL COMMENT 'municipality',
  `state` int(5) NOT NULL DEFAULT 0 COMMENT 'state/province index',
  `zip` varchar(255) DEFAULT NULL COMMENT 'postal code for physical address (no punctuation)',
  `country` int(4) NOT NULL DEFAULT 0 COMMENT 'country index',
  `latitude` decimal(11,8) NOT NULL DEFAULT 0.00000000 COMMENT 'latitude of account''s physical address',
  `longitude` decimal(11,8) NOT NULL DEFAULT 0.00000000 COMMENT 'longitude of account''s physical address',
  `notes` longtext DEFAULT NULL COMMENT 'miscellaneous notes about the user or the account',
  `tickle` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime to tickle an admin about this account',
  `activated` int(11) NOT NULL DEFAULT 0 COMMENT 'when was the account activated',
  `helper` bigint(20) NOT NULL DEFAULT 0 COMMENT 'account that invited this person or company',
  `iCode` int(11) NOT NULL DEFAULT 0 COMMENT 'sequence number of helper invitation',
  `signed` int(11) NOT NULL DEFAULT 0 COMMENT 'when did this person sign the Common Good Agreement',
  `signedBy` varchar(60) DEFAULT NULL COMMENT 'who signed the agreement (on behalf of the account)',
  `savingsAdd` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'chosen amount to hold as savings, beyond rewards',
  `saveWeekly` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'chosen amount to increase minimum (target balance) by, weekly',
  `floor` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'negative credit line',
  `minimum` decimal(11,2) DEFAULT NULL COMMENT 'chosen target balance (for automatic refills)',
  `crumbs` decimal(6,3) NOT NULL DEFAULT 0.000 COMMENT 'percentage of each transaction to donate to CG',
  `backing` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'amount account-holder chose to back',
  `backingDate` bigint(20) NOT NULL DEFAULT 0 COMMENT 'date account-holder started backing',
  `backingNext` decimal(11,2) DEFAULT NULL COMMENT 'lower backing amount for the next year',
  `food` decimal(6,3) NOT NULL DEFAULT 0.000 COMMENT 'percentage of each food purchase to donate to the food fund',
  `balance` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'balance, not including rewards (cached)',
  `rewards` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'total incentive rewards to date (cached)',
  `committed` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'amount committed (for donations to CGF)',
  `risk` float DEFAULT NULL COMMENT 'today''s suspiciousness rating',
  `risks` bigint(20) unsigned NOT NULL DEFAULT 0 COMMENT 'list of risk factors',
  `trust` float NOT NULL DEFAULT 0 COMMENT 'how much this person is trusted by others in the community (0 for companies)',
  `stats` longtext DEFAULT NULL COMMENT 'account statistics',
  `notices` text DEFAULT NULL COMMENT 'when to send what kind of notice',
  `lastip` varchar(39) DEFAULT NULL COMMENT 'latest IP address used',
  `preid` bigint(20) NOT NULL DEFAULT 0 COMMENT 'signup record ID',
  `special` longtext DEFAULT NULL COMMENT 'special transient data',
  `source` mediumtext DEFAULT NULL COMMENT 'how did the member hear about us?',
  PRIMARY KEY (`uid`,`deleted`),
  KEY `access` (`access`),
  KEY `created` (`created`),
  KEY `mail` (`email`),
  KEY `picture` (`picture`),
  KEY `name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Stores user data.';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `zip3`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `zip3` (
  `id` varchar(3) NOT NULL COMMENT 'first 3 digits of Zip Code',
  `region` varchar(255) DEFAULT NULL COMMENT 'city or region within a state',
  `state` varchar(2) DEFAULT NULL COMMENT 'state abbreviation',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='meaning of first 3 digits of Zip Codes';
/*!40101 SET character_set_client = @saved_cs_client */;
DROP TABLE IF EXISTS `zips`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `zips` (
  `zip` int(11) NOT NULL,
  `type` varchar(8) NOT NULL,
  `decommissioned` bit(1) NOT NULL,
  `primary_city` varchar(27) NOT NULL,
  `acceptable_cities` varchar(282) DEFAULT NULL,
  `unacceptable_cities` varchar(2208) DEFAULT NULL,
  `state` varchar(2) NOT NULL,
  `county` varchar(39) DEFAULT NULL,
  `timezone` varchar(30) DEFAULT NULL,
  `area_codes` varchar(35) DEFAULT NULL,
  `world_region` varchar(2) NOT NULL,
  `country` varchar(2) NOT NULL,
  `latitude` decimal(6,2) NOT NULL,
  `longitude` decimal(7,2) NOT NULL,
  `irs_estimated_population_2015` int(11) NOT NULL,
  PRIMARY KEY (`zip`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'NO_AUTO_VALUE_ON_ZERO' */ ;
/*!50003 DROP FUNCTION IF EXISTS `perYear` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_general_ci */ ;
DELIMITER ;;
CREATE FUNCTION `perYear`(`period` ENUM('once','day','week','month','quarter','year','forever'), `periods` INT(11)) RETURNS tinyint(4)
    DETERMINISTIC
RETURN (CASE period
        WHEN 'forever' THEN 0
        WHEN 'year' THEN 1
        WHEN 'quarter' THEN 4
        WHEN 'month' THEN 12
        WHEN 'week' THEN 365.25/7
        WHEN 'day' THEN 365.25
        WHEN 'once' THEN 0
        ELSE -1
      END) / periods
;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50001 DROP VIEW IF EXISTS `ancestors`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8mb3 */;
/*!50001 SET character_set_results     = utf8mb3 */;
/*!50001 SET collation_connection      = utf8mb3_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 SQL SECURITY DEFINER */
/*!50001 VIEW `ancestors` AS with recursive ancestors as (select `r_industries`.`iid` AS `base`,`r_industries`.`industry` AS `baseIndustry`,`r_industries`.`iid` AS `iid`,`r_industries`.`industry` AS `industry`,`r_industries`.`parent` AS `parent` from `r_industries` union all select `a`.`base` AS `base`,`a`.`baseIndustry` AS `baseIndustry`,`p`.`iid` AS `iid`,`p`.`industry` AS `industry`,`p`.`parent` AS `parent` from (`r_industries` `p` join `ancestors` `a` on(`p`.`iid` = `a`.`parent`)))select `ancestors`.`base` AS `base`,`ancestors`.`baseIndustry` AS `baseIndustry`,`ancestors`.`iid` AS `ancestor`,`ancestors`.`industry` AS `ancestorIndustry` from `ancestors` */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;
/*!50001 DROP VIEW IF EXISTS `descendants`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8mb3 */;
/*!50001 SET character_set_results     = utf8mb3 */;
/*!50001 SET collation_connection      = utf8mb3_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 SQL SECURITY DEFINER */
/*!50001 VIEW `descendants` AS with recursive descendants as (select `r_industries`.`iid` AS `base`,`r_industries`.`industry` AS `baseIndustry`,`r_industries`.`iid` AS `iid`,`r_industries`.`industry` AS `industry` from `r_industries` union all select `d`.`base` AS `base`,`d`.`baseIndustry` AS `baseIndustry`,`c`.`iid` AS `iid`,`c`.`industry` AS `industry` from (`r_industries` `c` join `descendants` `d` on(`d`.`iid` = `c`.`parent`)))select `descendants`.`base` AS `base`,`descendants`.`baseIndustry` AS `baseIndustry`,`descendants`.`iid` AS `descendant`,`descendants`.`industry` AS `descendantIndustry` from `descendants` */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;
/*!50001 DROP VIEW IF EXISTS `tx_credits`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8mb3 */;
/*!50001 SET character_set_results     = utf8mb3 */;
/*!50001 SET collation_connection      = utf8mb3_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 SQL SECURITY DEFINER */
/*!50001 VIEW `tx_credits` AS select `tx_requests_all`.`nvid` AS `id`,`tx_requests_all`.`created` AS `created`,`tx_requests_all`.`payee` AS `fromUid`,`tx_requests_all`.`payer` AS `toUid`,`tx_requests_all`.`amount` AS `amount`,`tx_requests_all`.`status` AS `xid`,`tx_requests_all`.`purpose` AS `purpose` from `tx_requests_all` where `tx_requests_all`.`amount` < 0 */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;
/*!50001 DROP VIEW IF EXISTS `tx_disputes`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8mb3 */;
/*!50001 SET character_set_results     = utf8mb3 */;
/*!50001 SET collation_connection      = utf8mb3_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 SQL SECURITY DEFINER */
/*!50001 VIEW `tx_disputes` AS select `tx_disputes_all`.`id` AS `id`,`tx_disputes_all`.`xid` AS `xid`,`tx_disputes_all`.`uid` AS `uid`,`tx_disputes_all`.`agentUid` AS `agentUid`,`tx_disputes_all`.`reason` AS `reason`,`tx_disputes_all`.`status` AS `status`,`tx_disputes_all`.`deleted` AS `deleted` from `tx_disputes_all` where `tx_disputes_all`.`deleted` is null */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;
/*!50001 DROP VIEW IF EXISTS `tx_entries`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8mb3 */;
/*!50001 SET character_set_results     = utf8mb3 */;
/*!50001 SET collation_connection      = utf8mb3_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 SQL SECURITY DEFINER */
/*!50001 VIEW `tx_entries` AS select `tx_entries_all`.`id` AS `id`,`tx_entries_all`.`xid` AS `xid`,`tx_entries_all`.`entryType` AS `entryType`,`tx_entries_all`.`amount` AS `amount`,`tx_entries_all`.`uid` AS `uid`,`tx_entries_all`.`agentUid` AS `agentUid`,`tx_entries_all`.`description` AS `description`,`tx_entries_all`.`cat` AS `cat`,`tx_entries_all`.`relType` AS `relType`,`tx_entries_all`.`relatedId` AS `relatedId`,`tx_entries_all`.`rule` AS `rule` from `tx_entries_all` where `tx_entries_all`.`deleted` is null */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;
/*!50001 DROP VIEW IF EXISTS `tx_entries_o`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8mb3 */;
/*!50001 SET character_set_results     = utf8mb3 */;
/*!50001 SET collation_connection      = utf8mb3_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 SQL SECURITY DEFINER */
/*!50001 VIEW `tx_entries_o` AS select `tx_entries`.`id` AS `id`,`tx_entries`.`xid` AS `xid`,`tx_entries`.`entryType` AS `entryType`,`tx_entries`.`amount` AS `amount`,`tx_entries`.`uid` AS `uid`,`tx_entries`.`agentUid` AS `agentUid`,`tx_entries`.`description` AS `description`,`tx_entries`.`cat` AS `cat`,`tx_entries`.`relType` AS `relType`,`tx_entries`.`relatedId` AS `relatedId`,`tx_entries`.`rule` AS `rule` from `tx_entries` order by abs(`tx_entries`.`id`),`tx_entries`.`id` */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;
/*!50001 DROP VIEW IF EXISTS `tx_hdrs`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8mb3 */;
/*!50001 SET character_set_results     = utf8mb3 */;
/*!50001 SET collation_connection      = utf8mb3_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 SQL SECURITY DEFINER */
/*!50001 VIEW `tx_hdrs` AS select `tx_hdrs_all`.`xid` AS `xid`,`tx_hdrs_all`.`actorId` AS `actorId`,`tx_hdrs_all`.`actorAgentId` AS `actorAgentId`,`tx_hdrs_all`.`flags` AS `flags`,`tx_hdrs_all`.`channel` AS `channel`,`tx_hdrs_all`.`boxId` AS `boxId`,`tx_hdrs_all`.`goods` AS `goods`,`tx_hdrs_all`.`risk` AS `risk`,`tx_hdrs_all`.`risks` AS `risks`,`tx_hdrs_all`.`recursId` AS `recursId`,`tx_hdrs_all`.`reversesXid` AS `reversesXid`,`tx_hdrs_all`.`created` AS `created` from `tx_hdrs_all` where `tx_hdrs_all`.`deleted` is null */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;
/*!50001 DROP VIEW IF EXISTS `tx_requests`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8mb3 */;
/*!50001 SET character_set_results     = utf8mb3 */;
/*!50001 SET collation_connection      = utf8mb3_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 SQL SECURITY DEFINER */
/*!50001 VIEW `tx_requests` AS select `tx_requests_all`.`nvid` AS `nvid`,`tx_requests_all`.`status` AS `status`,`tx_requests_all`.`amount` AS `amount`,`tx_requests_all`.`payer` AS `payer`,`tx_requests_all`.`payee` AS `payee`,`tx_requests_all`.`goods` AS `goods`,`tx_requests_all`.`purpose` AS `purpose`,`tx_requests_all`.`cat` AS `cat`,`tx_requests_all`.`flags` AS `flags`,`tx_requests_all`.`data` AS `data`,`tx_requests_all`.`reversesXid` AS `reversesXid`,`tx_requests_all`.`recursId` AS `recursId`,`tx_requests_all`.`created` AS `created`,`tx_requests_all`.`deleted` AS `deleted` from `tx_requests_all` where `tx_requests_all`.`deleted` is null and `tx_requests_all`.`amount` > 0 */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;
/*!50001 DROP VIEW IF EXISTS `txs`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8mb3 */;
/*!50001 SET character_set_results     = utf8mb3 */;
/*!50001 SET collation_connection      = utf8mb3_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 SQL SECURITY DEFINER */
/*!50001 VIEW `txs` AS select distinct `t`.`xid` AS `xid`,`t`.`actorId` AS `actorId`,`t`.`actorAgentId` AS `actorAgentId`,`t`.`flags` AS `flags`,`t`.`channel` AS `channel`,`t`.`boxId` AS `boxId`,`t`.`goods` AS `goods`,`t`.`risk` AS `risk`,`t`.`risks` AS `risks`,`t`.`recursId` AS `recursId`,`t`.`reversesXid` AS `reversesXid`,`t`.`created` AS `created`,`e2`.`entryType` AS `type`,`e2`.`amount` AS `amt`,`e2`.`rule` AS `rule`,`e2`.`relType` AS `relType`,`e2`.`relatedId` AS `rel`,`e2`.`id` AS `eid`,`e2`.`description` AS `for2`,`e2`.`uid` AS `uid2`,`e2`.`agentUid` AS `agt2`,`e2`.`cat` AS `cat2`,`e1`.`description` AS `for1`,`e1`.`uid` AS `uid1`,`e1`.`agentUid` AS `agt1`,`e1`.`cat` AS `cat1` from ((`tx_hdrs` `t` join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`rule` AS `rule` from (`tx_entries` `e` join `tx_entries` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 1 and `e`.`id` < 0 or `e`.`entryType` = 2 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` < 0 or `e`.`entryType` = 3 and `e2`.`entryType` = 1 and `e2`.`id` < 0 or `e`.`entryType` = 4 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` < 0 or `e`.`entryType` = 5 and `e`.`uid` in (256,257) and `e2`.`uid` not in (256,257) or `e`.`entryType` = 7 and `e`.`uid` = 255 and `e2`.`uid` <> 255) `e1` on(`t`.`xid` = `e1`.`xid`)) join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`rule` AS `rule` from (`tx_entries` `e` join `tx_entries` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 1 and `e`.`id` > 0 or `e`.`entryType` = 2 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` > 0 or `e`.`entryType` = 3 and `e2`.`entryType` = 1 and `e2`.`id` > 0 or `e`.`entryType` = 4 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` > 0 or `e`.`entryType` = 5 and `e`.`uid` not in (256,257) and `e2`.`uid` in (256,257) or `e`.`entryType` = 7 and `e`.`uid` <> 255 and `e2`.`uid` = 255) `e2` on(`t`.`xid` = `e2`.`xid`)) where `e1`.`id` = -`e2`.`id` and `e2`.`id` > 0 */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;
/*!50001 DROP VIEW IF EXISTS `txs2_bank`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8mb3 */;
/*!50001 SET character_set_results     = utf8mb3 */;
/*!50001 SET collation_connection      = utf8mb3_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 SQL SECURITY DEFINER */
/*!50001 VIEW `txs2_bank` AS select `txs2`.`txid` AS `txid`,`txs2`.`amount` AS `amount`,`txs2`.`payee` AS `payee`,`txs2`.`created` AS `created`,`txs2`.`completed` AS `completed`,`txs2`.`deposit` AS `deposit`,`txs2`.`bankAccount` AS `bankAccount`,`txs2`.`isSavings` AS `isSavings`,`txs2`.`risk` AS `risk`,`txs2`.`risks` AS `risks`,`txs2`.`bankTxId` AS `bankTxId`,`txs2`.`channel` AS `channel`,`txs2`.`xid` AS `xid`,`txs2`.`pid` AS `pid` from `txs2` where `txs2`.`pid` is null */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;
/*!50001 DROP VIEW IF EXISTS `txs2_outer`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8mb3 */;
/*!50001 SET character_set_results     = utf8mb3 */;
/*!50001 SET collation_connection      = utf8mb3_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 SQL SECURITY DEFINER */
/*!50001 VIEW `txs2_outer` AS select `txs2`.`txid` AS `txid`,`txs2`.`amount` AS `amount`,`txs2`.`payee` AS `payee`,`txs2`.`created` AS `created`,`txs2`.`completed` AS `completed`,`txs2`.`deposit` AS `deposit`,`txs2`.`bankAccount` AS `bankAccount`,`txs2`.`isSavings` AS `isSavings`,`txs2`.`risk` AS `risk`,`txs2`.`risks` AS `risks`,`txs2`.`bankTxId` AS `bankTxId`,`txs2`.`channel` AS `channel`,`txs2`.`xid` AS `xid`,`txs2`.`pid` AS `pid` from `txs2` where `txs2`.`pid` is not null */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;
/*!50001 DROP VIEW IF EXISTS `txs_all`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8mb3 */;
/*!50001 SET character_set_results     = utf8mb3 */;
/*!50001 SET collation_connection      = utf8mb3_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 SQL SECURITY DEFINER */
/*!50001 VIEW `txs_all` AS select distinct `t`.`xid` AS `xid`,`t`.`actorId` AS `actorId`,`t`.`actorAgentId` AS `actorAgentId`,`t`.`flags` AS `flags`,`t`.`channel` AS `channel`,`t`.`boxId` AS `boxId`,`t`.`goods` AS `goods`,`t`.`risk` AS `risk`,`t`.`risks` AS `risks`,`t`.`recursId` AS `recursId`,`t`.`reversesXid` AS `reversesXid`,`t`.`created` AS `created`,`e2`.`entryType` AS `type`,`e2`.`amount` AS `amt`,`e2`.`rule` AS `rule`,`e2`.`relType` AS `relType`,`e2`.`relatedId` AS `rel`,`e2`.`id` AS `eid`,`e2`.`description` AS `for2`,`e2`.`uid` AS `uid2`,`e2`.`agentUid` AS `agt2`,`e2`.`cat` AS `cat2`,`e1`.`description` AS `for1`,`e1`.`uid` AS `uid1`,`e1`.`agentUid` AS `agt1`,`e1`.`cat` AS `cat1` from ((`tx_hdrs` `t` join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`rule` AS `rule` from (`tx_entries` `e` join `tx_entries` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 1 and `e`.`id` < 0 or `e`.`entryType` = 2 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` < 0 or `e`.`entryType` = 3 and `e2`.`entryType` = 1 and `e2`.`id` < 0 or `e`.`entryType` = 4 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` < 0 or `e`.`entryType` = 5 and `e`.`uid` in (256,257) and `e2`.`uid` not in (256,257) or `e`.`entryType` = 7 and `e`.`uid` = 255 and `e2`.`uid` <> 255 or `e`.`entryType` = 0 and if(`e`.`uid` in (256,257) and `e2`.`uid` in (256,257),`e`.`amount` < 0,`e`.`uid` in (256,257))) `e1` on(`t`.`xid` = `e1`.`xid`)) join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`rule` AS `rule` from (`tx_entries` `e` join `tx_entries` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 1 and `e`.`id` > 0 or `e`.`entryType` = 2 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` > 0 or `e`.`entryType` = 3 and `e2`.`entryType` = 1 and `e2`.`id` > 0 or `e`.`entryType` = 4 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` > 0 or `e`.`entryType` = 5 and `e`.`uid` not in (256,257) and `e2`.`uid` in (256,257) or `e`.`entryType` = 7 and `e`.`uid` <> 255 and `e2`.`uid` = 255 or `e`.`entryType` = 0 and if(`e`.`uid` in (256,257) and `e2`.`uid` in (256,257),`e`.`amount` > 0,`e2`.`uid` in (256,257))) `e2` on(`t`.`xid` = `e2`.`xid`)) where `e1`.`id` = -`e2`.`id` and `e2`.`id` > 0 */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;
/*!50001 DROP VIEW IF EXISTS `txs_aux`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8mb3 */;
/*!50001 SET character_set_results     = utf8mb3 */;
/*!50001 SET collation_connection      = utf8mb3_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 SQL SECURITY DEFINER */
/*!50001 VIEW `txs_aux` AS select distinct `t`.`xid` AS `xid`,`t`.`actorId` AS `actorId`,`t`.`actorAgentId` AS `actorAgentId`,`t`.`flags` AS `flags`,`t`.`channel` AS `channel`,`t`.`boxId` AS `boxId`,`t`.`goods` AS `goods`,`t`.`risk` AS `risk`,`t`.`risks` AS `risks`,`t`.`recursId` AS `recursId`,`t`.`reversesXid` AS `reversesXid`,`t`.`created` AS `created`,`e2`.`entryType` AS `type`,`e2`.`amount` AS `amt`,`e2`.`rule` AS `rule`,`e2`.`relType` AS `relType`,`e2`.`relatedId` AS `rel`,`e2`.`id` AS `eid`,`e2`.`description` AS `for2`,`e2`.`uid` AS `uid2`,`e2`.`agentUid` AS `agt2`,`e2`.`cat` AS `cat2`,`e1`.`description` AS `for1`,`e1`.`uid` AS `uid1`,`e1`.`agentUid` AS `agt1`,`e1`.`cat` AS `cat1` from ((`tx_hdrs` `t` join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`rule` AS `rule` from (`tx_entries` `e` join `tx_entries` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 2 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` < 0) `e1` on(`t`.`xid` = `e1`.`xid`)) join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`rule` AS `rule` from (`tx_entries` `e` join `tx_entries` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 2 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` > 0) `e2` on(`t`.`xid` = `e2`.`xid`)) where `e1`.`id` = -`e2`.`id` and `e2`.`id` > 0 */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;
/*!50001 DROP VIEW IF EXISTS `txs_bank`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8mb3 */;
/*!50001 SET character_set_results     = utf8mb3 */;
/*!50001 SET collation_connection      = utf8mb3_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 SQL SECURITY DEFINER */
/*!50001 VIEW `txs_bank` AS select distinct `t`.`xid` AS `xid`,`t`.`actorId` AS `actorId`,`t`.`actorAgentId` AS `actorAgentId`,`t`.`flags` AS `flags`,`t`.`channel` AS `channel`,`t`.`boxId` AS `boxId`,`t`.`goods` AS `goods`,`t`.`risk` AS `risk`,`t`.`risks` AS `risks`,`t`.`recursId` AS `recursId`,`t`.`reversesXid` AS `reversesXid`,`t`.`created` AS `created`,`e2`.`entryType` AS `type`,`e2`.`amount` AS `amt`,`e2`.`rule` AS `rule`,`e2`.`relType` AS `relType`,`e2`.`relatedId` AS `rel`,`e2`.`id` AS `eid`,`e2`.`description` AS `for2`,`e2`.`uid` AS `uid2`,`e2`.`agentUid` AS `agt2`,`e2`.`cat` AS `cat2`,`e1`.`description` AS `for1`,`e1`.`uid` AS `uid1`,`e1`.`agentUid` AS `agt1`,`e1`.`cat` AS `cat1` from ((`tx_hdrs` `t` join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`rule` AS `rule` from (`tx_entries` `e` join `tx_entries` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 5 and `e`.`uid` in (256,257) and `e2`.`uid` not in (256,257)) `e1` on(`t`.`xid` = `e1`.`xid`)) join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`rule` AS `rule` from (`tx_entries` `e` join `tx_entries` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 5 and `e`.`uid` not in (256,257) and `e2`.`uid` in (256,257)) `e2` on(`t`.`xid` = `e2`.`xid`)) where `e1`.`id` = -`e2`.`id` and `e2`.`id` > 0 */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;
/*!50001 DROP VIEW IF EXISTS `txs_bank_only`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8mb3 */;
/*!50001 SET character_set_results     = utf8mb3 */;
/*!50001 SET collation_connection      = utf8mb3_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 SQL SECURITY DEFINER */
/*!50001 VIEW `txs_bank_only` AS select distinct `t`.`xid` AS `xid`,`t`.`actorId` AS `actorId`,`t`.`actorAgentId` AS `actorAgentId`,`t`.`flags` AS `flags`,`t`.`channel` AS `channel`,`t`.`boxId` AS `boxId`,`t`.`goods` AS `goods`,`t`.`risk` AS `risk`,`t`.`risks` AS `risks`,`t`.`recursId` AS `recursId`,`t`.`reversesXid` AS `reversesXid`,`t`.`created` AS `created`,`e2`.`entryType` AS `type`,`e2`.`amount` AS `amt`,`e2`.`rule` AS `rule`,`e2`.`relType` AS `relType`,`e2`.`relatedId` AS `rel`,`e2`.`id` AS `eid`,`e2`.`description` AS `for2`,`e2`.`uid` AS `uid2`,`e2`.`agentUid` AS `agt2`,`e2`.`cat` AS `cat2`,`e1`.`description` AS `for1`,`e1`.`uid` AS `uid1`,`e1`.`agentUid` AS `agt1`,`e1`.`cat` AS `cat1` from ((`tx_hdrs` `t` join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`rule` AS `rule` from (`tx_entries` `e` join `tx_entries` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 0 and if(`e`.`uid` in (256,257) and `e2`.`uid` in (256,257),`e`.`amount` < 0,`e`.`uid` in (256,257))) `e1` on(`t`.`xid` = `e1`.`xid`)) join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`rule` AS `rule` from (`tx_entries` `e` join `tx_entries` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 0 and if(`e`.`uid` in (256,257) and `e2`.`uid` in (256,257),`e`.`amount` > 0,`e2`.`uid` in (256,257))) `e2` on(`t`.`xid` = `e2`.`xid`)) where `e1`.`id` = -`e2`.`id` and `e2`.`id` > 0 */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;
/*!50001 DROP VIEW IF EXISTS `txs_deleted`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8mb3 */;
/*!50001 SET character_set_results     = utf8mb3 */;
/*!50001 SET collation_connection      = utf8mb3_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 SQL SECURITY DEFINER */
/*!50001 VIEW `txs_deleted` AS select distinct `t`.`deleted` AS `deleted`,`t`.`xid` AS `xid`,`t`.`actorId` AS `actorId`,`t`.`actorAgentId` AS `actorAgentId`,`t`.`flags` AS `flags`,`t`.`channel` AS `channel`,`t`.`boxId` AS `boxId`,`t`.`goods` AS `goods`,`t`.`risk` AS `risk`,`t`.`risks` AS `risks`,`t`.`recursId` AS `recursId`,`t`.`reversesXid` AS `reversesXid`,`t`.`created` AS `created`,`e2`.`entryType` AS `type`,`e2`.`amount` AS `amt`,`e2`.`rule` AS `rule`,`e2`.`relType` AS `relType`,`e2`.`relatedId` AS `rel`,`e2`.`id` AS `eid`,`e2`.`description` AS `for2`,`e2`.`uid` AS `uid2`,`e2`.`agentUid` AS `agt2`,`e2`.`cat` AS `cat2`,`e1`.`description` AS `for1`,`e1`.`uid` AS `uid1`,`e1`.`agentUid` AS `agt1`,`e1`.`cat` AS `cat1` from ((`tx_hdrs_all` `t` join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`deleted` AS `deleted`,`e`.`rule` AS `rule` from (`tx_entries_all` `e` join `tx_entries_all` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 1 and `e`.`id` < 0 or `e`.`entryType` = 2 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` < 0 or `e`.`entryType` = 3 and `e2`.`entryType` = 1 and `e2`.`id` < 0 or `e`.`entryType` = 4 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` < 0 or `e`.`entryType` = 5 and `e`.`uid` in (256,257) and `e2`.`uid` not in (256,257) or `e`.`entryType` = 7 and `e`.`uid` = 255 and `e2`.`uid` <> 255 or `e`.`entryType` = 0 and if(`e`.`uid` in (256,257) and `e2`.`uid` in (256,257),`e`.`amount` < 0,`e`.`uid` in (256,257))) `e1` on(`t`.`xid` = `e1`.`xid`)) join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`deleted` AS `deleted`,`e`.`rule` AS `rule` from (`tx_entries_all` `e` join `tx_entries_all` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 1 and `e`.`id` > 0 or `e`.`entryType` = 2 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` > 0 or `e`.`entryType` = 3 and `e2`.`entryType` = 1 and `e2`.`id` > 0 or `e`.`entryType` = 4 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` > 0 or `e`.`entryType` = 5 and `e`.`uid` not in (256,257) and `e2`.`uid` in (256,257) or `e`.`entryType` = 7 and `e`.`uid` <> 255 and `e2`.`uid` = 255 or `e`.`entryType` = 0 and if(`e`.`uid` in (256,257) and `e2`.`uid` in (256,257),`e`.`amount` > 0,`e2`.`uid` in (256,257))) `e2` on(`t`.`xid` = `e2`.`xid`)) where `t`.`deleted` is not null and `e1`.`id` = -`e2`.`id` and `e2`.`id` > 0 */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;
/*!50001 DROP VIEW IF EXISTS `txs_ever`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8mb3 */;
/*!50001 SET character_set_results     = utf8mb3 */;
/*!50001 SET collation_connection      = utf8mb3_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 SQL SECURITY DEFINER */
/*!50001 VIEW `txs_ever` AS select distinct `t`.`deleted` AS `deleted`,`t`.`xid` AS `xid`,`t`.`actorId` AS `actorId`,`t`.`actorAgentId` AS `actorAgentId`,`t`.`flags` AS `flags`,`t`.`channel` AS `channel`,`t`.`boxId` AS `boxId`,`t`.`goods` AS `goods`,`t`.`risk` AS `risk`,`t`.`risks` AS `risks`,`t`.`recursId` AS `recursId`,`t`.`reversesXid` AS `reversesXid`,`t`.`created` AS `created`,`e2`.`entryType` AS `type`,`e2`.`amount` AS `amt`,`e2`.`rule` AS `rule`,`e2`.`relType` AS `relType`,`e2`.`relatedId` AS `rel`,`e2`.`id` AS `eid`,`e2`.`description` AS `for2`,`e2`.`uid` AS `uid2`,`e2`.`agentUid` AS `agt2`,`e2`.`cat` AS `cat2`,`e1`.`description` AS `for1`,`e1`.`uid` AS `uid1`,`e1`.`agentUid` AS `agt1`,`e1`.`cat` AS `cat1` from ((`tx_hdrs_all` `t` join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`deleted` AS `deleted`,`e`.`rule` AS `rule` from (`tx_entries_all` `e` join `tx_entries_all` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 1 and `e`.`id` < 0 or `e`.`entryType` = 2 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` < 0 or `e`.`entryType` = 3 and `e2`.`entryType` = 1 and `e2`.`id` < 0 or `e`.`entryType` = 4 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` < 0 or `e`.`entryType` = 5 and `e`.`uid` in (256,257) and `e2`.`uid` not in (256,257) or `e`.`entryType` = 7 and `e`.`uid` = 255 and `e2`.`uid` <> 255 or `e`.`entryType` = 0 and if(`e`.`uid` in (256,257) and `e2`.`uid` in (256,257),`e`.`amount` < 0,`e`.`uid` in (256,257))) `e1` on(`t`.`xid` = `e1`.`xid`)) join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`deleted` AS `deleted`,`e`.`rule` AS `rule` from (`tx_entries_all` `e` join `tx_entries_all` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 1 and `e`.`id` > 0 or `e`.`entryType` = 2 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` > 0 or `e`.`entryType` = 3 and `e2`.`entryType` = 1 and `e2`.`id` > 0 or `e`.`entryType` = 4 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` > 0 or `e`.`entryType` = 5 and `e`.`uid` not in (256,257) and `e2`.`uid` in (256,257) or `e`.`entryType` = 7 and `e`.`uid` <> 255 and `e2`.`uid` = 255 or `e`.`entryType` = 0 and if(`e`.`uid` in (256,257) and `e2`.`uid` in (256,257),`e`.`amount` > 0,`e2`.`uid` in (256,257))) `e2` on(`t`.`xid` = `e2`.`xid`)) where `e1`.`id` = -`e2`.`id` and `e2`.`id` > 0 */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;
/*!50001 DROP VIEW IF EXISTS `txs_noreverse`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8mb3 */;
/*!50001 SET character_set_results     = utf8mb3 */;
/*!50001 SET collation_connection      = utf8mb3_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 SQL SECURITY DEFINER */
/*!50001 VIEW `txs_noreverse` AS select `t`.`xid` AS `xid`,`t`.`actorId` AS `actorId`,`t`.`actorAgentId` AS `actorAgentId`,`t`.`flags` AS `flags`,`t`.`channel` AS `channel`,`t`.`boxId` AS `boxId`,`t`.`goods` AS `goods`,`t`.`risk` AS `risk`,`t`.`risks` AS `risks`,`t`.`recursId` AS `recursId`,`t`.`reversesXid` AS `reversesXid`,`t`.`created` AS `created`,`t`.`type` AS `type`,`t`.`amt` AS `amt`,`t`.`rule` AS `rule`,`t`.`relType` AS `relType`,`t`.`rel` AS `rel`,`t`.`eid` AS `eid`,`t`.`for2` AS `for2`,`t`.`uid2` AS `uid2`,`t`.`agt2` AS `agt2`,`t`.`cat2` AS `cat2`,`t`.`for1` AS `for1`,`t`.`uid1` AS `uid1`,`t`.`agt1` AS `agt1`,`t`.`cat1` AS `cat1` from `txs` `t` where `t`.`reversesXid` is null and !exists(select `tr`.`xid` from `tx_hdrs` `tr` where `tr`.`reversesXid` = `t`.`xid` limit 1) */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;
/*!50001 DROP VIEW IF EXISTS `txs_outer`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8mb3 */;
/*!50001 SET character_set_results     = utf8mb3 */;
/*!50001 SET collation_connection      = utf8mb3_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 SQL SECURITY DEFINER */
/*!50001 VIEW `txs_outer` AS select distinct `t`.`xid` AS `xid`,`t`.`actorId` AS `actorId`,`t`.`actorAgentId` AS `actorAgentId`,`t`.`flags` AS `flags`,`t`.`channel` AS `channel`,`t`.`boxId` AS `boxId`,`t`.`goods` AS `goods`,`t`.`risk` AS `risk`,`t`.`risks` AS `risks`,`t`.`recursId` AS `recursId`,`t`.`reversesXid` AS `reversesXid`,`t`.`created` AS `created`,`e2`.`entryType` AS `type`,`e2`.`amount` AS `amt`,`e2`.`rule` AS `rule`,`e2`.`relType` AS `relType`,`e2`.`relatedId` AS `rel`,`e2`.`id` AS `eid`,`e2`.`description` AS `for2`,`e2`.`uid` AS `uid2`,`e2`.`agentUid` AS `agt2`,`e2`.`cat` AS `cat2`,`e1`.`description` AS `for1`,`e1`.`uid` AS `uid1`,`e1`.`agentUid` AS `agt1`,`e1`.`cat` AS `cat1` from ((`tx_hdrs` `t` join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`rule` AS `rule` from (`tx_entries` `e` join `tx_entries` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 7 and `e`.`uid` = 255 and `e2`.`uid` <> 255) `e1` on(`t`.`xid` = `e1`.`xid`)) join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`rule` AS `rule` from (`tx_entries` `e` join `tx_entries` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 7 and `e`.`uid` <> 255 and `e2`.`uid` = 255) `e2` on(`t`.`xid` = `e2`.`xid`)) where `e1`.`id` = -`e2`.`id` and `e2`.`id` > 0 */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;
/*!50001 DROP VIEW IF EXISTS `txs_prime`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8mb3 */;
/*!50001 SET character_set_results     = utf8mb3 */;
/*!50001 SET collation_connection      = utf8mb3_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 SQL SECURITY DEFINER */
/*!50001 VIEW `txs_prime` AS select distinct `t`.`xid` AS `xid`,`t`.`actorId` AS `actorId`,`t`.`actorAgentId` AS `actorAgentId`,`t`.`flags` AS `flags`,`t`.`channel` AS `channel`,`t`.`boxId` AS `boxId`,`t`.`goods` AS `goods`,`t`.`risk` AS `risk`,`t`.`risks` AS `risks`,`t`.`recursId` AS `recursId`,`t`.`reversesXid` AS `reversesXid`,`t`.`created` AS `created`,`e2`.`entryType` AS `type`,`e2`.`amount` AS `amt`,`e2`.`rule` AS `rule`,`e2`.`relType` AS `relType`,`e2`.`relatedId` AS `rel`,`e2`.`id` AS `eid`,`e2`.`description` AS `for2`,`e2`.`uid` AS `uid2`,`e2`.`agentUid` AS `agt2`,`e2`.`cat` AS `cat2`,`e1`.`description` AS `for1`,`e1`.`uid` AS `uid1`,`e1`.`agentUid` AS `agt1`,`e1`.`cat` AS `cat1` from ((`tx_hdrs` `t` join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`rule` AS `rule` from (`tx_entries` `e` join `tx_entries` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 1 and `e`.`id` < 0) `e1` on(`t`.`xid` = `e1`.`xid`)) join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`rule` AS `rule` from (`tx_entries` `e` join `tx_entries` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 1 and `e`.`id` > 0) `e2` on(`t`.`xid` = `e2`.`xid`)) where `e1`.`id` = -`e2`.`id` and `e2`.`id` > 0 */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;
/*!50001 DROP VIEW IF EXISTS `txs_proper`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8mb3 */;
/*!50001 SET character_set_results     = utf8mb3 */;
/*!50001 SET collation_connection      = utf8mb3_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 SQL SECURITY DEFINER */
/*!50001 VIEW `txs_proper` AS select distinct `t`.`xid` AS `xid`,`t`.`actorId` AS `actorId`,`t`.`actorAgentId` AS `actorAgentId`,`t`.`flags` AS `flags`,`t`.`channel` AS `channel`,`t`.`boxId` AS `boxId`,`t`.`goods` AS `goods`,`t`.`risk` AS `risk`,`t`.`risks` AS `risks`,`t`.`recursId` AS `recursId`,`t`.`reversesXid` AS `reversesXid`,`t`.`created` AS `created`,`e2`.`entryType` AS `type`,`e2`.`amount` AS `amt`,`e2`.`rule` AS `rule`,`e2`.`relType` AS `relType`,`e2`.`relatedId` AS `rel`,`e2`.`id` AS `eid`,`e2`.`description` AS `for2`,`e2`.`uid` AS `uid2`,`e2`.`agentUid` AS `agt2`,`e2`.`cat` AS `cat2`,`e1`.`description` AS `for1`,`e1`.`uid` AS `uid1`,`e1`.`agentUid` AS `agt1`,`e1`.`cat` AS `cat1` from ((`tx_hdrs` `t` join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`rule` AS `rule` from (`tx_entries` `e` join `tx_entries` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 1 and `e`.`id` < 0 or `e`.`entryType` = 2 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` < 0 or `e`.`entryType` = 3 and `e2`.`entryType` = 1 and `e2`.`id` < 0 or `e`.`entryType` = 4 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` < 0) `e1` on(`t`.`xid` = `e1`.`xid`)) join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`rule` AS `rule` from (`tx_entries` `e` join `tx_entries` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 1 and `e`.`id` > 0 or `e`.`entryType` = 2 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` > 0 or `e`.`entryType` = 3 and `e2`.`entryType` = 1 and `e2`.`id` > 0 or `e`.`entryType` = 4 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` > 0) `e2` on(`t`.`xid` = `e2`.`xid`)) where `e1`.`id` = -`e2`.`id` and `e2`.`id` > 0 */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;
/*!50001 DROP VIEW IF EXISTS `txs_rebate`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8mb3 */;
/*!50001 SET character_set_results     = utf8mb3 */;
/*!50001 SET collation_connection      = utf8mb3_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 SQL SECURITY DEFINER */
/*!50001 VIEW `txs_rebate` AS select distinct `t`.`xid` AS `xid`,`t`.`actorId` AS `actorId`,`t`.`actorAgentId` AS `actorAgentId`,`t`.`flags` AS `flags`,`t`.`channel` AS `channel`,`t`.`boxId` AS `boxId`,`t`.`goods` AS `goods`,`t`.`risk` AS `risk`,`t`.`risks` AS `risks`,`t`.`recursId` AS `recursId`,`t`.`reversesXid` AS `reversesXid`,`t`.`created` AS `created`,`e2`.`entryType` AS `type`,`e2`.`amount` AS `amt`,`e2`.`rule` AS `rule`,`e2`.`relType` AS `relType`,`e2`.`relatedId` AS `rel`,`e2`.`id` AS `eid`,`e2`.`description` AS `for2`,`e2`.`uid` AS `uid2`,`e2`.`agentUid` AS `agt2`,`e2`.`cat` AS `cat2`,`e1`.`description` AS `for1`,`e1`.`uid` AS `uid1`,`e1`.`agentUid` AS `agt1`,`e1`.`cat` AS `cat1` from ((`tx_hdrs` `t` join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`rule` AS `rule` from (`tx_entries` `e` join `tx_entries` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 3 and `e2`.`entryType` = 1 and `e2`.`id` < 0) `e1` on(`t`.`xid` = `e1`.`xid`)) join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`rule` AS `rule` from (`tx_entries` `e` join `tx_entries` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 3 and `e2`.`entryType` = 1 and `e2`.`id` > 0) `e2` on(`t`.`xid` = `e2`.`xid`)) where `e1`.`id` = -`e2`.`id` and `e2`.`id` > 0 */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;
/*!50001 DROP VIEW IF EXISTS `txs_usd_fee`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8mb3 */;
/*!50001 SET character_set_results     = utf8mb3 */;
/*!50001 SET collation_connection      = utf8mb3_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 SQL SECURITY DEFINER */
/*!50001 VIEW `txs_usd_fee` AS select distinct `t`.`xid` AS `xid`,`t`.`actorId` AS `actorId`,`t`.`actorAgentId` AS `actorAgentId`,`t`.`flags` AS `flags`,`t`.`channel` AS `channel`,`t`.`boxId` AS `boxId`,`t`.`goods` AS `goods`,`t`.`risk` AS `risk`,`t`.`risks` AS `risks`,`t`.`recursId` AS `recursId`,`t`.`reversesXid` AS `reversesXid`,`t`.`created` AS `created`,`e2`.`entryType` AS `type`,`e2`.`amount` AS `amt`,`e2`.`rule` AS `rule`,`e2`.`relType` AS `relType`,`e2`.`relatedId` AS `rel`,`e2`.`id` AS `eid`,`e2`.`description` AS `for2`,`e2`.`uid` AS `uid2`,`e2`.`agentUid` AS `agt2`,`e2`.`cat` AS `cat2`,`e1`.`description` AS `for1`,`e1`.`uid` AS `uid1`,`e1`.`agentUid` AS `agt1`,`e1`.`cat` AS `cat1` from ((`tx_hdrs` `t` join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`rule` AS `rule` from (`tx_entries` `e` join `tx_entries` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 4 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` < 0) `e1` on(`t`.`xid` = `e1`.`xid`)) join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`rule` AS `rule` from (`tx_entries` `e` join `tx_entries` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 4 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` > 0) `e2` on(`t`.`xid` = `e2`.`xid`)) where `e1`.`id` = -`e2`.`id` and `e2`.`id` > 0 */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

LOCK TABLES `block` WRITE;
/*!40000 ALTER TABLE `block` DISABLE KEYS */;
INSERT INTO `block` VALUES (423,'rweb','accounts','rcredits',1,0,'accounts',0,0,'','',1);
INSERT INTO `block` VALUES (424,'rweb','footer','rcredits',1,0,'footer',0,0,'','',-1);
INSERT INTO `block` VALUES (425,'rweb','accounts','seven',0,0,'-1',0,0,'','',1);
INSERT INTO `block` VALUES (426,'rweb','footer','seven',0,0,'-1',0,0,'','',-1);
/*!40000 ALTER TABLE `block` ENABLE KEYS */;
UNLOCK TABLES;

LOCK TABLES `phinxlog` WRITE;
/*!40000 ALTER TABLE `phinxlog` DISABLE KEYS */;
INSERT INTO `phinxlog` VALUES (20190213232929,'NewAgreementAndBacking','2021-04-22 00:11:32','2021-04-22 00:11:32',0);
INSERT INTO `phinxlog` VALUES (20190311170655,'PayWithCgLink','2021-04-22 00:11:32','2021-04-22 00:11:32',0);
INSERT INTO `phinxlog` VALUES (20190322132020,'FixSetlocus','2021-04-22 00:11:32','2021-04-22 00:11:32',0);
INSERT INTO `phinxlog` VALUES (20190322132821,'OneMetric','2021-04-22 00:11:32','2021-04-22 00:11:32',0);
INSERT INTO `phinxlog` VALUES (20190506153458,'BalanceSheet','2021-04-22 00:11:32','2021-04-22 00:11:32',0);
INSERT INTO `phinxlog` VALUES (20190507010000,'CreateTableTxHdrsAll','2021-04-22 00:11:32','2021-04-22 00:11:32',0);
INSERT INTO `phinxlog` VALUES (20190507020000,'CreateTableTxEntriesAll','2021-04-22 00:11:32','2021-04-22 00:11:32',0);
INSERT INTO `phinxlog` VALUES (20190507030000,'AlterUsdTables','2021-04-22 00:11:32','2021-04-22 00:11:32',0);
INSERT INTO `phinxlog` VALUES (20190507040000,'CreateTableTxDisputesAll','2021-04-22 00:11:32','2021-04-22 00:11:32',0);
INSERT INTO `phinxlog` VALUES (20190507050000,'AddCanonicalAccounts','2021-04-22 00:11:32','2021-04-22 00:11:32',0);
INSERT INTO `phinxlog` VALUES (20190507060000,'ReformatTransactions','2021-04-22 00:11:32','2021-04-22 00:11:32',0);
INSERT INTO `phinxlog` VALUES (20190507070000,'CreateBalanceTriggers','2021-04-22 00:11:32','2021-04-22 00:11:32',0);
INSERT INTO `phinxlog` VALUES (20190507080000,'InitializeBalances','2021-04-22 00:11:32','2021-04-22 00:11:32',0);
INSERT INTO `phinxlog` VALUES (20190507090000,'RenameOldTxsTable','2021-04-22 00:11:32','2021-04-22 00:11:32',0);
INSERT INTO `phinxlog` VALUES (20190531232058,'PostSplitFixes','2021-04-22 00:11:32','2021-04-22 00:11:32',0);
INSERT INTO `phinxlog` VALUES (20190605135132,'FixRecurring','2021-04-22 00:11:32','2021-04-22 00:11:32',0);
INSERT INTO `phinxlog` VALUES (20190614035252,'SignupTracking','2021-04-22 00:11:32','2021-04-22 00:11:32',0);
INSERT INTO `phinxlog` VALUES (20190618174054,'FoodPercents','2021-04-22 00:11:32','2021-04-22 00:11:32',0);
INSERT INTO `phinxlog` VALUES (20190622131859,'SimplifyTxs','2021-04-22 00:11:32','2021-04-22 00:11:32',0);
INSERT INTO `phinxlog` VALUES (20190710134016,'Invest','2021-04-22 00:11:32','2021-04-22 00:11:33',0);
INSERT INTO `phinxlog` VALUES (20190727234655,'RestrictedCoupons','2021-04-22 00:11:33','2021-04-22 00:11:33',0);
INSERT INTO `phinxlog` VALUES (20190802200257,'PiecemealDiscounts','2021-04-22 00:11:33','2021-04-22 00:11:33',0);
INSERT INTO `phinxlog` VALUES (20190804180734,'Whence','2021-04-22 00:11:33','2021-04-22 00:11:33',0);
INSERT INTO `phinxlog` VALUES (20190810142735,'CouponFixes','2021-04-22 00:11:33','2021-04-22 00:11:33',0);
INSERT INTO `phinxlog` VALUES (20190820133520,'Ezbiz','2021-04-22 00:11:33','2021-04-22 00:11:33',0);
INSERT INTO `phinxlog` VALUES (20191002173152,'SimpleSignup','2021-04-22 00:11:33','2021-04-22 00:11:33',0);
INSERT INTO `phinxlog` VALUES (20191021133320,'EasyTweaks','2021-04-22 00:11:33','2021-04-22 00:11:33',0);
INSERT INTO `phinxlog` VALUES (20200224161017,'MakeIndustriesRecursive','2021-04-22 00:11:33','2021-04-22 00:11:33',0);
INSERT INTO `phinxlog` VALUES (20200303120100,'CreateTableUGroups','2021-04-22 00:11:33','2021-04-22 00:11:33',0);
INSERT INTO `phinxlog` VALUES (20200303120200,'CreateTableUGroupies','2021-04-22 00:11:33','2021-04-22 00:11:33',0);
INSERT INTO `phinxlog` VALUES (20200303120300,'CreateTableTxTemplates','2021-04-22 00:11:33','2021-04-22 00:11:33',0);
INSERT INTO `phinxlog` VALUES (20200303120400,'CreateTableTxRules','2021-04-22 00:11:33','2021-04-22 00:11:33',0);
INSERT INTO `phinxlog` VALUES (20200303120500,'AddFieldsToTxEntriesAll','2021-04-22 00:11:33','2021-04-22 00:11:33',0);
INSERT INTO `phinxlog` VALUES (20200303120600,'MigrateCoupons','2021-04-22 00:11:33','2021-04-22 00:11:33',0);
INSERT INTO `phinxlog` VALUES (20200303120700,'HideCouponRelatedTables','2021-04-22 00:11:33','2021-04-22 00:11:33',0);
INSERT INTO `phinxlog` VALUES (20200313220601,'FreeOffers','2021-04-22 00:11:33','2021-04-22 00:11:33',0);
INSERT INTO `phinxlog` VALUES (20200318025417,'PostTweaks','2021-04-22 00:11:33','2021-04-22 00:11:33',0);
INSERT INTO `phinxlog` VALUES (20200318181411,'AddPostCat','2021-04-22 00:11:33','2021-04-22 00:11:33',0);
INSERT INTO `phinxlog` VALUES (20200319170844,'PostGeosearch','2021-04-22 00:11:33','2021-04-22 00:11:33',0);
INSERT INTO `phinxlog` VALUES (20200325023154,'PostFormat','2021-04-22 00:11:33','2021-04-22 00:11:33',0);
INSERT INTO `phinxlog` VALUES (20200327225059,'PostModerate','2021-04-22 00:11:33','2021-04-22 00:11:33',0);
INSERT INTO `phinxlog` VALUES (20200328192849,'PostTouchups2','2021-04-22 00:11:33','2021-04-22 00:11:33',0);
INSERT INTO `phinxlog` VALUES (20200330015207,'PostBugs','2021-04-22 00:11:33','2021-04-22 00:11:34',0);
INSERT INTO `phinxlog` VALUES (20200405033914,'PostTips','2021-04-22 00:11:34','2021-04-22 00:11:34',0);
INSERT INTO `phinxlog` VALUES (20200428134609,'RecursCentral','2021-04-22 00:11:34','2021-04-22 00:11:34',0);
INSERT INTO `phinxlog` VALUES (20200428174926,'PerYearFunction','2021-04-22 00:11:34','2021-04-22 00:11:34',0);
INSERT INTO `phinxlog` VALUES (20200507233810,'PostNotices','2021-04-22 00:11:34','2021-04-22 00:11:34',0);
INSERT INTO `phinxlog` VALUES (20200511134354,'Please','2021-04-22 00:11:34','2021-04-22 00:11:34',0);
INSERT INTO `phinxlog` VALUES (20200526000522,'PhotoPermission','2021-04-22 00:11:34','2021-04-22 00:11:34',0);
INSERT INTO `phinxlog` VALUES (20200531124652,'InviteAll','2021-04-22 00:11:34','2021-04-22 00:11:34',0);
INSERT INTO `phinxlog` VALUES (20200608161335,'InviteOverhaul','2021-04-22 00:11:34','2021-04-22 00:11:34',0);
INSERT INTO `phinxlog` VALUES (20200702225507,'PushEndorse','2021-04-22 00:11:34','2021-04-22 00:11:34',0);
INSERT INTO `phinxlog` VALUES (20200707130318,'RenewBacking','2021-04-22 00:11:34','2021-04-22 00:11:34',0);
INSERT INTO `phinxlog` VALUES (20200903125352,'Notices','2021-04-22 00:11:34','2021-04-22 00:11:34',0);
INSERT INTO `phinxlog` VALUES (20200917034029,'PostExpand','2021-04-22 00:11:34','2021-04-22 00:11:34',0);
INSERT INTO `phinxlog` VALUES (20200929073504,'Sweep','2021-04-22 00:11:34','2021-04-22 00:11:34',0);
INSERT INTO `phinxlog` VALUES (20201012050311,'Mb4','2021-04-22 00:11:34','2021-04-22 00:11:37',0);
INSERT INTO `phinxlog` VALUES (20201018043604,'NoBank','2021-04-22 00:11:37','2021-04-22 00:11:37',0);
INSERT INTO `phinxlog` VALUES (20201028121138,'ShortSignup','2021-04-22 00:11:37','2021-04-22 00:11:37',0);
INSERT INTO `phinxlog` VALUES (20201124082459,'Cttys','2021-04-22 00:11:37','2021-04-22 00:11:37',0);
INSERT INTO `phinxlog` VALUES (20201201053609,'PgpDeviceId','2021-04-22 00:11:37','2021-04-22 00:11:37',0);
INSERT INTO `phinxlog` VALUES (20210518071537,'Unpay','2024-02-02 20:58:03','2024-02-02 20:58:03',0);
INSERT INTO `phinxlog` VALUES (20210527100150,'Sponsor','2024-02-02 20:58:03','2024-02-02 20:58:03',0);
INSERT INTO `phinxlog` VALUES (20210607111225,'Invoices','2024-02-02 20:58:03','2024-02-02 20:58:03',0);
INSERT INTO `phinxlog` VALUES (20210623090725,'Logo','2024-02-02 20:58:03','2024-02-02 20:58:03',0);
INSERT INTO `phinxlog` VALUES (20210702120044,'Txs2','2024-02-02 20:58:03','2024-02-02 20:58:04',0);
INSERT INTO `phinxlog` VALUES (20210706122107,'Fbo2','2024-02-02 20:58:04','2024-02-02 20:58:04',0);
INSERT INTO `phinxlog` VALUES (20210706123242,'MoveCat','2024-02-02 20:58:04','2024-02-02 20:58:05',0);
INSERT INTO `phinxlog` VALUES (20210706144217,'MakeOuter','2024-02-02 20:58:05','2024-02-02 20:58:05',0);
INSERT INTO `phinxlog` VALUES (20210710025233,'Fbo3','2024-02-02 20:58:05','2024-02-02 20:58:05',0);
INSERT INTO `phinxlog` VALUES (20210713110102,'Zips','2024-02-02 20:58:05','2024-02-02 20:58:10',0);
INSERT INTO `phinxlog` VALUES (20210725025306,'Founded','2024-02-02 20:58:10','2024-02-02 20:58:10',0);
INSERT INTO `phinxlog` VALUES (20210912053426,'Deletion','2024-02-02 20:58:10','2024-02-02 20:58:10',0);
INSERT INTO `phinxlog` VALUES (20211108030520,'SimplerSignup','2024-02-02 20:58:10','2024-02-02 20:58:10',0);
INSERT INTO `phinxlog` VALUES (20211209042914,'Supers','2024-02-02 20:58:10','2024-02-02 20:58:10',0);
INSERT INTO `phinxlog` VALUES (20211230110407,'Admins','2024-02-02 20:58:10','2024-02-02 20:58:10',0);
INSERT INTO `phinxlog` VALUES (20220211045249,'CcGifts','2024-02-02 20:58:10','2024-02-02 20:58:10',0);
INSERT INTO `phinxlog` VALUES (20220222023529,'MinimalAdmin','2024-02-02 20:58:10','2024-02-02 20:58:11',0);
INSERT INTO `phinxlog` VALUES (20220318124031,'QbAccounts','2024-02-02 20:58:11','2024-02-02 20:58:11',0);
INSERT INTO `phinxlog` VALUES (20220408092039,'Cg2qb','2024-02-02 20:58:11','2024-02-02 20:58:11',0);
INSERT INTO `phinxlog` VALUES (20220412044909,'SetCats','2024-02-02 20:58:11','2024-02-02 20:58:11',0);
INSERT INTO `phinxlog` VALUES (20220504104223,'QbTxs','2024-02-02 20:58:11','2024-02-02 20:58:11',0);
INSERT INTO `phinxlog` VALUES (20220512080235,'QbCttyFund','2024-02-02 20:58:11','2024-02-02 20:58:11',0);
INSERT INTO `phinxlog` VALUES (20220521071358,'Qbo3','2024-02-02 20:58:11','2024-02-02 20:58:12',0);
INSERT INTO `phinxlog` VALUES (20220529025942,'CatTest','2024-02-02 20:58:12','2024-02-02 20:58:12',0);
INSERT INTO `phinxlog` VALUES (20220606070942,'Clickup2','2024-02-02 20:58:12','2024-02-02 20:58:12',0);
INSERT INTO `phinxlog` VALUES (20220613051718,'Sql','2024-02-02 20:58:12','2024-02-02 20:58:12',0);
INSERT INTO `phinxlog` VALUES (20220717035808,'Loyalty','2024-02-02 20:58:12','2024-02-02 20:58:13',0);
INSERT INTO `phinxlog` VALUES (20221021104444,'DescLen','2024-02-02 20:58:13','2024-02-02 20:58:13',0);
INSERT INTO `phinxlog` VALUES (20230224101130,'Comments','2024-02-02 20:58:13','2024-02-02 20:58:13',0);
INSERT INTO `phinxlog` VALUES (20230527033654,'BadsMsg','2024-02-02 20:58:13','2024-02-02 20:58:13',0);
INSERT INTO `phinxlog` VALUES (20230527051322,'BadNegAmt','2024-02-02 20:58:13','2024-02-02 20:58:13',0);
INSERT INTO `phinxlog` VALUES (20230811023357,'Thermometer','2024-02-02 20:58:13','2024-02-02 20:58:13',0);
INSERT INTO `phinxlog` VALUES (20230816065722,'CreateSponsee','2024-02-02 20:58:13','2024-02-02 20:58:13',0);
INSERT INTO `phinxlog` VALUES (20230930015403,'Cap','2024-02-02 20:58:13','2024-02-02 20:58:13',0);
INSERT INTO `phinxlog` VALUES (20231018071151,'BigBadField','2024-02-02 20:58:13','2024-02-02 20:58:13',0);
INSERT INTO `phinxlog` VALUES (20240316033926,'SponsorTitle','2026-11-14 18:22:46','2026-11-14 18:22:46',0);
INSERT INTO `phinxlog` VALUES (20240320120615,'SponsorTitleX','2026-11-14 18:22:46','2026-11-14 18:22:46',0);
INSERT INTO `phinxlog` VALUES (20240811114530,'TxsDeleted','2026-11-14 18:22:46','2026-11-14 18:22:47',0);
INSERT INTO `phinxlog` VALUES (20240816034949,'BigQbId','2026-11-14 18:22:47','2026-11-14 18:22:47',0);
INSERT INTO `phinxlog` VALUES (20240821072803,'Focuses','2026-11-14 18:22:47','2026-11-14 18:22:47',0);
INSERT INTO `phinxlog` VALUES (20240904124231,'OFAC','2026-11-14 18:22:47','2026-11-14 18:22:47',0);
INSERT INTO `phinxlog` VALUES (20240910072610,'Stripe','2026-11-14 18:22:47','2026-11-14 18:22:47',0);
INSERT INTO `phinxlog` VALUES (20241002063236,'Invest','2026-11-14 18:22:47','2026-11-14 18:22:47',0);
INSERT INTO `phinxlog` VALUES (20241016081859,'GeneralizeChanges','2026-11-14 18:22:47','2026-11-14 18:22:47',0);
INSERT INTO `phinxlog` VALUES (20241109015357,'Nicks','2026-11-14 18:22:47','2026-11-14 18:22:48',0);
INSERT INTO `phinxlog` VALUES (20241205033647,'GiftThanks','2026-11-14 18:22:48','2026-11-14 18:22:48',0);
INSERT INTO `phinxlog` VALUES (20241223000000,'GiftThanksX','2026-11-14 18:22:48','2026-11-14 18:22:48',0);
INSERT INTO `phinxlog` VALUES (20251215023413,'Grants','2026-11-14 18:22:48','2026-11-14 18:22:48',0);
INSERT INTO `phinxlog` VALUES (20251227043917,'GrantsByCheck','2026-11-14 18:22:48','2026-11-14 18:22:48',0);
INSERT INTO `phinxlog` VALUES (20260518054016,'FSExpenseMarkers','2026-11-14 18:22:48','2026-11-14 18:22:48',0);
INSERT INTO `phinxlog` VALUES (20260722193000,'GrantsAddFund','2026-11-14 18:22:48','2026-11-14 18:22:48',0);
INSERT INTO `phinxlog` VALUES (20260722193200,'UrelationsTxNotify','2026-11-14 18:22:48','2026-11-14 18:22:48',0);
INSERT INTO `phinxlog` VALUES (20260901180000,'GrantsAddDriveFileId','2026-11-14 18:22:48','2026-11-14 18:22:48',0);
INSERT INTO `phinxlog` VALUES (20261024033407,'CorrectionsToRecentMigrations','2026-11-14 18:22:48','2026-11-14 18:22:48',0);
INSERT INTO `phinxlog` VALUES (20261107093900,'ResetCodes','2026-11-14 18:22:48','2026-11-14 18:22:48',0);
/*!40000 ALTER TABLE `phinxlog` ENABLE KEYS */;
UNLOCK TABLES;

LOCK TABLES `post_cats` WRITE;
/*!40000 ALTER TABLE `post_cats` DISABLE KEYS */;
INSERT INTO `post_cats` VALUES (1,'food',100);
INSERT INTO `post_cats` VALUES (2,'housing',200);
INSERT INTO `post_cats` VALUES (4,'health',400);
INSERT INTO `post_cats` VALUES (5,'travel/rides',1930);
INSERT INTO `post_cats` VALUES (6,'delivery',600);
INSERT INTO `post_cats` VALUES (7,'childcare',700);
INSERT INTO `post_cats` VALUES (9,'animal care',900);
INSERT INTO `post_cats` VALUES (10,'cleaning',1910);
INSERT INTO `post_cats` VALUES (11,'legal',1100);
INSERT INTO `post_cats` VALUES (13,'fellowship',1300);
INSERT INTO `post_cats` VALUES (14,'muscle/labor',1400);
INSERT INTO `post_cats` VALUES (16,'technology',1600);
INSERT INTO `post_cats` VALUES (17,'info/training',1700);
INSERT INTO `post_cats` VALUES (19,'finance/money',1900);
INSERT INTO `post_cats` VALUES (20,'other',2000);
/*!40000 ALTER TABLE `post_cats` ENABLE KEYS */;
UNLOCK TABLES;

LOCK TABLES `r_areas` WRITE;
/*!40000 ALTER TABLE `r_areas` DISABLE KEYS */;
INSERT INTO `r_areas` VALUES ('201','NJ');
INSERT INTO `r_areas` VALUES ('202','DC');
INSERT INTO `r_areas` VALUES ('203','CT');
INSERT INTO `r_areas` VALUES ('204','MB');
INSERT INTO `r_areas` VALUES ('205','AL');
INSERT INTO `r_areas` VALUES ('206','WA');
INSERT INTO `r_areas` VALUES ('207','ME');
INSERT INTO `r_areas` VALUES ('208','ID');
INSERT INTO `r_areas` VALUES ('209','CA');
INSERT INTO `r_areas` VALUES ('210','TX');
INSERT INTO `r_areas` VALUES ('212','NY');
INSERT INTO `r_areas` VALUES ('213','CA');
INSERT INTO `r_areas` VALUES ('214','TX');
INSERT INTO `r_areas` VALUES ('215','PA');
INSERT INTO `r_areas` VALUES ('216','OH');
INSERT INTO `r_areas` VALUES ('217','IL');
INSERT INTO `r_areas` VALUES ('218','MN');
INSERT INTO `r_areas` VALUES ('219','IN');
INSERT INTO `r_areas` VALUES ('224','IL');
INSERT INTO `r_areas` VALUES ('225','LA');
INSERT INTO `r_areas` VALUES ('226','ON');
INSERT INTO `r_areas` VALUES ('228','MS');
INSERT INTO `r_areas` VALUES ('229','GA');
INSERT INTO `r_areas` VALUES ('231','MI');
INSERT INTO `r_areas` VALUES ('234','OH');
INSERT INTO `r_areas` VALUES ('239','FL');
INSERT INTO `r_areas` VALUES ('240','MD');
INSERT INTO `r_areas` VALUES ('242','Bahamas');
INSERT INTO `r_areas` VALUES ('246','Barbados');
INSERT INTO `r_areas` VALUES ('248','MI');
INSERT INTO `r_areas` VALUES ('250','BC');
INSERT INTO `r_areas` VALUES ('251','AL');
INSERT INTO `r_areas` VALUES ('252','NC');
INSERT INTO `r_areas` VALUES ('253','WA');
INSERT INTO `r_areas` VALUES ('254','TX');
INSERT INTO `r_areas` VALUES ('256','AL');
INSERT INTO `r_areas` VALUES ('260','IN');
INSERT INTO `r_areas` VALUES ('262','WI');
INSERT INTO `r_areas` VALUES ('264','Anguilla');
INSERT INTO `r_areas` VALUES ('267','PA');
INSERT INTO `r_areas` VALUES ('268','Antigua and Barbuda');
INSERT INTO `r_areas` VALUES ('269','MI');
INSERT INTO `r_areas` VALUES ('270','KY');
INSERT INTO `r_areas` VALUES ('276','VA');
INSERT INTO `r_areas` VALUES ('281','TX');
INSERT INTO `r_areas` VALUES ('284','British Virgin Islands');
INSERT INTO `r_areas` VALUES ('289','ON');
INSERT INTO `r_areas` VALUES ('301','MD');
INSERT INTO `r_areas` VALUES ('302','DE');
INSERT INTO `r_areas` VALUES ('303','CO');
INSERT INTO `r_areas` VALUES ('304','WV');
INSERT INTO `r_areas` VALUES ('305','FL');
INSERT INTO `r_areas` VALUES ('306','SK');
INSERT INTO `r_areas` VALUES ('307','WY');
INSERT INTO `r_areas` VALUES ('308','NE');
INSERT INTO `r_areas` VALUES ('309','IL');
INSERT INTO `r_areas` VALUES ('310','CA');
INSERT INTO `r_areas` VALUES ('312','IL');
INSERT INTO `r_areas` VALUES ('313','MI');
INSERT INTO `r_areas` VALUES ('314','MO');
INSERT INTO `r_areas` VALUES ('315','NY');
INSERT INTO `r_areas` VALUES ('316','KS');
INSERT INTO `r_areas` VALUES ('317','IN');
INSERT INTO `r_areas` VALUES ('318','LA');
INSERT INTO `r_areas` VALUES ('319','IA');
INSERT INTO `r_areas` VALUES ('320','MN');
INSERT INTO `r_areas` VALUES ('321','FL');
INSERT INTO `r_areas` VALUES ('323','CA');
INSERT INTO `r_areas` VALUES ('325','TX');
INSERT INTO `r_areas` VALUES ('330','OH');
INSERT INTO `r_areas` VALUES ('334','AL');
INSERT INTO `r_areas` VALUES ('336','NC');
INSERT INTO `r_areas` VALUES ('337','LA');
INSERT INTO `r_areas` VALUES ('339','MA');
INSERT INTO `r_areas` VALUES ('340','USVI');
INSERT INTO `r_areas` VALUES ('345','Cayman Islands');
INSERT INTO `r_areas` VALUES ('347','NY');
INSERT INTO `r_areas` VALUES ('351','MA');
INSERT INTO `r_areas` VALUES ('352','FL');
INSERT INTO `r_areas` VALUES ('360','WA');
INSERT INTO `r_areas` VALUES ('361','TX');
INSERT INTO `r_areas` VALUES ('386','FL');
INSERT INTO `r_areas` VALUES ('401','RI');
INSERT INTO `r_areas` VALUES ('402','NE');
INSERT INTO `r_areas` VALUES ('403','AB');
INSERT INTO `r_areas` VALUES ('404','GA');
INSERT INTO `r_areas` VALUES ('405','OK');
INSERT INTO `r_areas` VALUES ('406','MT');
INSERT INTO `r_areas` VALUES ('407','FL');
INSERT INTO `r_areas` VALUES ('408','CA');
INSERT INTO `r_areas` VALUES ('409','TX');
INSERT INTO `r_areas` VALUES ('410','MD');
INSERT INTO `r_areas` VALUES ('412','PA');
INSERT INTO `r_areas` VALUES ('413','MA');
INSERT INTO `r_areas` VALUES ('414','WI');
INSERT INTO `r_areas` VALUES ('415','CA');
INSERT INTO `r_areas` VALUES ('416','ON');
INSERT INTO `r_areas` VALUES ('417','MO');
INSERT INTO `r_areas` VALUES ('418','QC');
INSERT INTO `r_areas` VALUES ('419','OH');
INSERT INTO `r_areas` VALUES ('423','TN');
INSERT INTO `r_areas` VALUES ('424','CA');
INSERT INTO `r_areas` VALUES ('425','WA');
INSERT INTO `r_areas` VALUES ('430','TX');
INSERT INTO `r_areas` VALUES ('432','TX');
INSERT INTO `r_areas` VALUES ('434','VA');
INSERT INTO `r_areas` VALUES ('435','UT');
INSERT INTO `r_areas` VALUES ('438','QC');
INSERT INTO `r_areas` VALUES ('440','OH');
INSERT INTO `r_areas` VALUES ('441','Bermuda');
INSERT INTO `r_areas` VALUES ('443','MD');
INSERT INTO `r_areas` VALUES ('450','QC');
INSERT INTO `r_areas` VALUES ('456','Reserved');
INSERT INTO `r_areas` VALUES ('469','TX');
INSERT INTO `r_areas` VALUES ('473','Grenada');
INSERT INTO `r_areas` VALUES ('478','GA');
INSERT INTO `r_areas` VALUES ('479','AR');
INSERT INTO `r_areas` VALUES ('480','AZ');
INSERT INTO `r_areas` VALUES ('484','PA');
INSERT INTO `r_areas` VALUES ('500','Reserved');
INSERT INTO `r_areas` VALUES ('501','AR');
INSERT INTO `r_areas` VALUES ('502','KY');
INSERT INTO `r_areas` VALUES ('503','OR');
INSERT INTO `r_areas` VALUES ('504','LA');
INSERT INTO `r_areas` VALUES ('505','NM');
INSERT INTO `r_areas` VALUES ('506','NB');
INSERT INTO `r_areas` VALUES ('507','MN');
INSERT INTO `r_areas` VALUES ('508','MA');
INSERT INTO `r_areas` VALUES ('509','WA');
INSERT INTO `r_areas` VALUES ('510','CA');
INSERT INTO `r_areas` VALUES ('512','TX');
INSERT INTO `r_areas` VALUES ('513','OH');
INSERT INTO `r_areas` VALUES ('514','QC');
INSERT INTO `r_areas` VALUES ('515','IA');
INSERT INTO `r_areas` VALUES ('516','NY');
INSERT INTO `r_areas` VALUES ('517','MI');
INSERT INTO `r_areas` VALUES ('518','NY');
INSERT INTO `r_areas` VALUES ('519','ON');
INSERT INTO `r_areas` VALUES ('520','AZ');
INSERT INTO `r_areas` VALUES ('530','CA');
INSERT INTO `r_areas` VALUES ('540','VA');
INSERT INTO `r_areas` VALUES ('541','OR');
INSERT INTO `r_areas` VALUES ('551','NJ');
INSERT INTO `r_areas` VALUES ('559','CA');
INSERT INTO `r_areas` VALUES ('561','FL');
INSERT INTO `r_areas` VALUES ('562','CA');
INSERT INTO `r_areas` VALUES ('563','IA');
INSERT INTO `r_areas` VALUES ('567','OH');
INSERT INTO `r_areas` VALUES ('570','PA');
INSERT INTO `r_areas` VALUES ('571','VA');
INSERT INTO `r_areas` VALUES ('573','MO');
INSERT INTO `r_areas` VALUES ('574','IN');
INSERT INTO `r_areas` VALUES ('580','OK');
INSERT INTO `r_areas` VALUES ('585','NY');
INSERT INTO `r_areas` VALUES ('586','MI');
INSERT INTO `r_areas` VALUES ('600','Reserved');
INSERT INTO `r_areas` VALUES ('601','MS');
INSERT INTO `r_areas` VALUES ('602','AZ');
INSERT INTO `r_areas` VALUES ('603','NH');
INSERT INTO `r_areas` VALUES ('604','BC');
INSERT INTO `r_areas` VALUES ('605','SD');
INSERT INTO `r_areas` VALUES ('606','KY');
INSERT INTO `r_areas` VALUES ('607','NY');
INSERT INTO `r_areas` VALUES ('608','WI');
INSERT INTO `r_areas` VALUES ('609','NJ');
INSERT INTO `r_areas` VALUES ('610','PA');
INSERT INTO `r_areas` VALUES ('612','MN');
INSERT INTO `r_areas` VALUES ('613','ON');
INSERT INTO `r_areas` VALUES ('614','OH');
INSERT INTO `r_areas` VALUES ('615','TN');
INSERT INTO `r_areas` VALUES ('616','MI');
INSERT INTO `r_areas` VALUES ('617','MA');
INSERT INTO `r_areas` VALUES ('618','IL');
INSERT INTO `r_areas` VALUES ('619','CA');
INSERT INTO `r_areas` VALUES ('620','KS');
INSERT INTO `r_areas` VALUES ('623','AZ');
INSERT INTO `r_areas` VALUES ('626','CA');
INSERT INTO `r_areas` VALUES ('630','IL');
INSERT INTO `r_areas` VALUES ('631','NY');
INSERT INTO `r_areas` VALUES ('636','MO');
INSERT INTO `r_areas` VALUES ('641','IA');
INSERT INTO `r_areas` VALUES ('646','NY');
INSERT INTO `r_areas` VALUES ('647','ON');
INSERT INTO `r_areas` VALUES ('649','Turks & Caicos Islands');
INSERT INTO `r_areas` VALUES ('650','CA');
INSERT INTO `r_areas` VALUES ('651','MN');
INSERT INTO `r_areas` VALUES ('660','MO');
INSERT INTO `r_areas` VALUES ('661','CA');
INSERT INTO `r_areas` VALUES ('662','MS');
INSERT INTO `r_areas` VALUES ('664','Montserrat');
INSERT INTO `r_areas` VALUES ('670','MP');
INSERT INTO `r_areas` VALUES ('671','GU');
INSERT INTO `r_areas` VALUES ('678','GA');
INSERT INTO `r_areas` VALUES ('682','TX');
INSERT INTO `r_areas` VALUES ('684','AS');
INSERT INTO `r_areas` VALUES ('700','Reserved');
INSERT INTO `r_areas` VALUES ('701','ND');
INSERT INTO `r_areas` VALUES ('702','NV');
INSERT INTO `r_areas` VALUES ('703','VA');
INSERT INTO `r_areas` VALUES ('704','NC');
INSERT INTO `r_areas` VALUES ('705','ON');
INSERT INTO `r_areas` VALUES ('706','GA');
INSERT INTO `r_areas` VALUES ('707','CA');
INSERT INTO `r_areas` VALUES ('708','IL');
INSERT INTO `r_areas` VALUES ('709','NL');
INSERT INTO `r_areas` VALUES ('710','US Government');
INSERT INTO `r_areas` VALUES ('712','IA');
INSERT INTO `r_areas` VALUES ('713','TX');
INSERT INTO `r_areas` VALUES ('714','CA');
INSERT INTO `r_areas` VALUES ('715','WI');
INSERT INTO `r_areas` VALUES ('716','NY');
INSERT INTO `r_areas` VALUES ('717','PA');
INSERT INTO `r_areas` VALUES ('718','NY');
INSERT INTO `r_areas` VALUES ('719','CO');
INSERT INTO `r_areas` VALUES ('720','CO');
INSERT INTO `r_areas` VALUES ('724','PA');
INSERT INTO `r_areas` VALUES ('727','FL');
INSERT INTO `r_areas` VALUES ('731','TN');
INSERT INTO `r_areas` VALUES ('732','NJ');
INSERT INTO `r_areas` VALUES ('734','MI');
INSERT INTO `r_areas` VALUES ('740','OH');
INSERT INTO `r_areas` VALUES ('754','FL');
INSERT INTO `r_areas` VALUES ('757','VA');
INSERT INTO `r_areas` VALUES ('758','St. Lucia');
INSERT INTO `r_areas` VALUES ('760','CA');
INSERT INTO `r_areas` VALUES ('762','GA');
INSERT INTO `r_areas` VALUES ('763','MN');
INSERT INTO `r_areas` VALUES ('765','IN');
INSERT INTO `r_areas` VALUES ('767','Dominica');
INSERT INTO `r_areas` VALUES ('769','MS');
INSERT INTO `r_areas` VALUES ('770','GA');
INSERT INTO `r_areas` VALUES ('772','FL');
INSERT INTO `r_areas` VALUES ('773','IL');
INSERT INTO `r_areas` VALUES ('774','MA');
INSERT INTO `r_areas` VALUES ('775','NV');
INSERT INTO `r_areas` VALUES ('778','BC');
INSERT INTO `r_areas` VALUES ('780','AB');
INSERT INTO `r_areas` VALUES ('781','MA');
INSERT INTO `r_areas` VALUES ('784','St. Vincent & Grenadines');
INSERT INTO `r_areas` VALUES ('785','KS');
INSERT INTO `r_areas` VALUES ('786','FL');
INSERT INTO `r_areas` VALUES ('787','PR');
INSERT INTO `r_areas` VALUES ('800','Toll Free');
INSERT INTO `r_areas` VALUES ('801','UT');
INSERT INTO `r_areas` VALUES ('802','VT');
INSERT INTO `r_areas` VALUES ('803','SC');
INSERT INTO `r_areas` VALUES ('804','VA');
INSERT INTO `r_areas` VALUES ('805','CA');
INSERT INTO `r_areas` VALUES ('806','TX');
INSERT INTO `r_areas` VALUES ('807','ON');
INSERT INTO `r_areas` VALUES ('808','HI');
INSERT INTO `r_areas` VALUES ('809','Dominican Republic');
INSERT INTO `r_areas` VALUES ('810','MI');
INSERT INTO `r_areas` VALUES ('812','IN');
INSERT INTO `r_areas` VALUES ('813','FL');
INSERT INTO `r_areas` VALUES ('814','PA');
INSERT INTO `r_areas` VALUES ('815','IL');
INSERT INTO `r_areas` VALUES ('816','MO');
INSERT INTO `r_areas` VALUES ('817','TX');
INSERT INTO `r_areas` VALUES ('818','CA');
INSERT INTO `r_areas` VALUES ('819','QC');
INSERT INTO `r_areas` VALUES ('828','NC');
INSERT INTO `r_areas` VALUES ('829','Dominican Republic');
INSERT INTO `r_areas` VALUES ('830','TX');
INSERT INTO `r_areas` VALUES ('831','CA');
INSERT INTO `r_areas` VALUES ('832','TX');
INSERT INTO `r_areas` VALUES ('843','SC');
INSERT INTO `r_areas` VALUES ('845','NY');
INSERT INTO `r_areas` VALUES ('847','IL');
INSERT INTO `r_areas` VALUES ('848','NJ');
INSERT INTO `r_areas` VALUES ('850','FL');
INSERT INTO `r_areas` VALUES ('856','NJ');
INSERT INTO `r_areas` VALUES ('857','MA');
INSERT INTO `r_areas` VALUES ('858','CA');
INSERT INTO `r_areas` VALUES ('859','KY');
INSERT INTO `r_areas` VALUES ('860','CT');
INSERT INTO `r_areas` VALUES ('862','NJ');
INSERT INTO `r_areas` VALUES ('863','FL');
INSERT INTO `r_areas` VALUES ('864','SC');
INSERT INTO `r_areas` VALUES ('865','TN');
INSERT INTO `r_areas` VALUES ('866','Toll Free');
INSERT INTO `r_areas` VALUES ('867','YT');
INSERT INTO `r_areas` VALUES ('868','Trinidad & Tobago');
INSERT INTO `r_areas` VALUES ('869','St. Kitts & Nevis');
INSERT INTO `r_areas` VALUES ('870','AR');
INSERT INTO `r_areas` VALUES ('876','Jamaica');
INSERT INTO `r_areas` VALUES ('877','Toll Free');
INSERT INTO `r_areas` VALUES ('878','PA');
INSERT INTO `r_areas` VALUES ('888','Toll Free');
INSERT INTO `r_areas` VALUES ('900','Toll Calls');
INSERT INTO `r_areas` VALUES ('901','TN');
INSERT INTO `r_areas` VALUES ('902','NS');
INSERT INTO `r_areas` VALUES ('903','TX');
INSERT INTO `r_areas` VALUES ('904','FL');
INSERT INTO `r_areas` VALUES ('905','ON');
INSERT INTO `r_areas` VALUES ('906','MI');
INSERT INTO `r_areas` VALUES ('907','AK');
INSERT INTO `r_areas` VALUES ('908','NJ');
INSERT INTO `r_areas` VALUES ('909','CA');
INSERT INTO `r_areas` VALUES ('910','NC');
INSERT INTO `r_areas` VALUES ('912','GA');
INSERT INTO `r_areas` VALUES ('913','KS');
INSERT INTO `r_areas` VALUES ('914','NY');
INSERT INTO `r_areas` VALUES ('915','TX');
INSERT INTO `r_areas` VALUES ('916','CA');
INSERT INTO `r_areas` VALUES ('917','NY');
INSERT INTO `r_areas` VALUES ('918','OK');
INSERT INTO `r_areas` VALUES ('919','NC');
INSERT INTO `r_areas` VALUES ('920','WI');
INSERT INTO `r_areas` VALUES ('925','CA');
INSERT INTO `r_areas` VALUES ('928','AZ');
INSERT INTO `r_areas` VALUES ('931','TN');
INSERT INTO `r_areas` VALUES ('936','TX');
INSERT INTO `r_areas` VALUES ('937','OH');
INSERT INTO `r_areas` VALUES ('939','PR');
INSERT INTO `r_areas` VALUES ('940','TX');
INSERT INTO `r_areas` VALUES ('941','FL');
INSERT INTO `r_areas` VALUES ('947','MI');
INSERT INTO `r_areas` VALUES ('949','CA');
INSERT INTO `r_areas` VALUES ('951','CA');
INSERT INTO `r_areas` VALUES ('952','MN');
INSERT INTO `r_areas` VALUES ('954','FL');
INSERT INTO `r_areas` VALUES ('956','TX');
INSERT INTO `r_areas` VALUES ('970','CO');
INSERT INTO `r_areas` VALUES ('971','OR');
INSERT INTO `r_areas` VALUES ('972','TX');
INSERT INTO `r_areas` VALUES ('973','NJ');
INSERT INTO `r_areas` VALUES ('978','MA');
INSERT INTO `r_areas` VALUES ('979','TX');
INSERT INTO `r_areas` VALUES ('980','NC');
INSERT INTO `r_areas` VALUES ('985','LA');
INSERT INTO `r_areas` VALUES ('989','MI');
/*!40000 ALTER TABLE `r_areas` ENABLE KEYS */;
UNLOCK TABLES;

LOCK TABLES `r_banks` WRITE;
/*!40000 ALTER TABLE `r_banks` DISABLE KEYS */;
INSERT INTO `r_banks` VALUES (211870281,0,11000015,1,'032608',0,'Greenfield Co-op Bank','63 Fed St Box 1345','Greenfield','MA','013010000','4137720293','1','1');
/*!40000 ALTER TABLE `r_banks` ENABLE KEYS */;
UNLOCK TABLES;

LOCK TABLES `r_countries` WRITE;
/*!40000 ALTER TABLE `r_countries` DISABLE KEYS */;
INSERT INTO `r_countries` VALUES (1001,'Afghanistan','AF',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1002,'Albania','AL',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1003,'Algeria','DZ',NULL,NULL,NULL,NULL,3,0);
INSERT INTO `r_countries` VALUES (1004,'American Samoa','AS',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1005,'Andorra','AD',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1006,'Angola','AO',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1007,'Anguilla','AI',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1008,'Antarctica','AQ',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1009,'Antigua and Barbuda','AG',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1010,'Argentina','AR',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1011,'Armenia','AM',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1012,'Aruba','AW',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1013,'Australia','AU',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1014,'Austria','AT',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1015,'Azerbaijan','AZ',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1016,'Bahrain','BH',NULL,NULL,NULL,NULL,3,0);
INSERT INTO `r_countries` VALUES (1017,'Bangladesh','BD',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1018,'Barbados','BB',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1019,'Belarus','BY',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1020,'Belgium','BE',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1021,'Belize','BZ',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1022,'Benin','BJ',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1023,'Bermuda','BM',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1024,'Bhutan','BT',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1025,'Bolivia','BO',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1026,'Bosnia and Herzegovina','BA',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1027,'Botswana','BW',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1028,'Bouvet Island','BV',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1029,'Brazil','BR',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1030,'British Indian Ocean Territory','IO',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1031,'Virgin Islands, British','VG',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1032,'Brunei Darussalam','BN',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1033,'Bulgaria','BG',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1034,'Burkina Faso','BF',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1035,'Myanmar','MM',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1036,'Burundi','BI',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1037,'Cambodia','KH',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1038,'Cameroon','CM',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1039,'Canada','CA',NULL,NULL,NULL,NULL,2,1);
INSERT INTO `r_countries` VALUES (1040,'Cape Verde','CV',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1041,'Cayman Islands','KY',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1042,'Central African Republic','CF',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1043,'Chad','TD',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1044,'Chile','CL',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1045,'China','CN',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1046,'Christmas Island','CX',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1047,'Cocos (Keeling) Islands','CC',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1048,'Colombia','CO',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1049,'Comoros','KM',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1050,'Congo, The Democratic Republic of the','CD',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1051,'Congo','CG',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1052,'Cook Islands','CK',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1053,'Costa Rica','CR',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1054,'CÃ´te d\'Ivoire','CI',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1055,'Croatia','HR',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1056,'Cuba','CU',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1057,'Cyprus','CY',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1058,'Czech Republic','CZ',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1059,'Denmark','DK',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1060,'Djibouti','DJ',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1061,'Dominica','DM',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1062,'Dominican Republic','DO',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1063,'Timor-Leste','TL',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1064,'Ecuador','EC',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1065,'Egypt','EG',NULL,NULL,NULL,NULL,3,0);
INSERT INTO `r_countries` VALUES (1066,'El Salvador','SV',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1067,'Equatorial Guinea','GQ',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1068,'Eritrea','ER',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1069,'Estonia','EE',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1070,'Ethiopia','ET',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1072,'Falkland Islands (Malvinas)','FK',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1073,'Faroe Islands','FO',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1074,'Fiji','FJ',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1075,'Finland','FI',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1076,'France','FR',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1077,'French Guiana','GF',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1078,'French Polynesia','PF',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1079,'French Southern Territories','TF',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1080,'Gabon','GA',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1081,'Georgia','GE',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1082,'Germany','DE',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1083,'Ghana','GH',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1084,'Gibraltar','GI',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1085,'Greece','GR',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1086,'Greenland','GL',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1087,'Grenada','GD',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1088,'Guadeloupe','GP',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1089,'Guam','GU',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1090,'Guatemala','GT',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1091,'Guinea','GN',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1092,'Guinea-Bissau','GW',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1093,'Guyana','GY',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1094,'Haiti','HT',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1095,'Heard Island and McDonald Islands','HM',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1096,'Holy See (Vatican City State)','VA',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1097,'Honduras','HN',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1098,'Hong Kong','HK',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1099,'Hungary','HU',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1100,'Iceland','IS',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1101,'India','IN',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1102,'Indonesia','ID',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1103,'Iran, Islamic Republic of','IR',NULL,NULL,NULL,NULL,3,0);
INSERT INTO `r_countries` VALUES (1104,'Iraq','IQ',NULL,NULL,NULL,NULL,3,0);
INSERT INTO `r_countries` VALUES (1105,'Ireland','IE',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1106,'Israel','IL',NULL,NULL,NULL,NULL,3,0);
INSERT INTO `r_countries` VALUES (1107,'Italy','IT',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1108,'Jamaica','JM',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1109,'Japan','JP',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1110,'Jordan','JO',NULL,NULL,NULL,NULL,3,0);
INSERT INTO `r_countries` VALUES (1111,'Kazakhstan','KZ',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1112,'Kenya','KE',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1113,'Kiribati','KI',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1114,'Korea, Democratic People\'s Republic of','KP',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1115,'Korea, Republic of','KR',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1116,'Kuwait','KW',NULL,NULL,NULL,NULL,3,0);
INSERT INTO `r_countries` VALUES (1117,'Kyrgyzstan','KG',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1118,'Lao People\'s Democratic Republic','LA',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1119,'Latvia','LV',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1120,'Lebanon','LB',NULL,NULL,NULL,NULL,3,0);
INSERT INTO `r_countries` VALUES (1121,'Lesotho','LS',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1122,'Liberia','LR',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1123,'Libyan Arab Jamahiriya','LY',NULL,NULL,NULL,NULL,3,0);
INSERT INTO `r_countries` VALUES (1124,'Liechtenstein','LI',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1125,'Lithuania','LT',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1126,'Luxembourg','LU',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1127,'Macao','MO',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1128,'Macedonia, Republic of','MK',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1129,'Madagascar','MG',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1130,'Malawi','MW',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1131,'Malaysia','MY',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1132,'Maldives','MV',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1133,'Mali','ML',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1134,'Malta','MT',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1135,'Marshall Islands','MH',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1136,'Martinique','MQ',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1137,'Mauritania','MR',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1138,'Mauritius','MU',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1139,'Mayotte','YT',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1140,'Mexico','MX',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1141,'Micronesia, Federated States of','FM',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1142,'Moldova','MD',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1143,'Monaco','MC',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1144,'Mongolia','MN',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1145,'Montserrat','MS',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1146,'Morocco','MA',NULL,NULL,NULL,NULL,3,0);
INSERT INTO `r_countries` VALUES (1147,'Mozambique','MZ',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1148,'Namibia','NA',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1149,'Nauru','NR',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1150,'Nepal','NP',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1151,'Netherlands Antilles','AN',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1152,'Netherlands','NL',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1153,'New Caledonia','NC',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1154,'New Zealand','NZ',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1155,'Nicaragua','NI',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1156,'Niger','NE',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1157,'Nigeria','NG',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1158,'Niue','NU',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1159,'Norfolk Island','NF',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1160,'Northern Mariana Islands','MP',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1161,'Norway','NO',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1162,'Oman','OM',NULL,NULL,NULL,NULL,3,0);
INSERT INTO `r_countries` VALUES (1163,'Pakistan','PK',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1164,'Palau','PW',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1165,'Palestinian Territory, Occupied','PS',NULL,NULL,NULL,NULL,3,0);
INSERT INTO `r_countries` VALUES (1166,'Panama','PA',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1167,'Papua New Guinea','PG',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1168,'Paraguay','PY',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1169,'Peru','PE',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1170,'Philippines','PH',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1171,'Pitcairn','PN',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1172,'Poland','PL',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1173,'Portugal','PT',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1174,'Puerto Rico','PR',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1175,'Qatar','QA',NULL,NULL,NULL,NULL,3,0);
INSERT INTO `r_countries` VALUES (1176,'Romania','RO',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1177,'Russian Federation','RU',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1178,'Rwanda','RW',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1179,'Reunion','RE',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1180,'Saint Helena','SH',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1181,'Saint Kitts and Nevis','KN',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1182,'Saint Lucia','LC',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1183,'Saint Pierre and Miquelon','PM',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1184,'Saint Vincent and the Grenadines','VC',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1185,'Samoa','WS',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1186,'San Marino','SM',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1187,'Saudi Arabia','SA',NULL,NULL,NULL,NULL,3,0);
INSERT INTO `r_countries` VALUES (1188,'Senegal','SN',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1189,'Seychelles','SC',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1190,'Sierra Leone','SL',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1191,'Singapore','SG',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1192,'Slovakia','SK',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1193,'Slovenia','SI',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1194,'Solomon Islands','SB',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1195,'Somalia','SO',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1196,'South Africa','ZA',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1197,'South Georgia and the South Sandwich Islands','GS',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1198,'Spain','ES',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1199,'Sri Lanka','LK',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1200,'Sudan','SD',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1201,'Suriname','SR',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1202,'Svalbard and Jan Mayen','SJ',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1203,'Swaziland','SZ',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1204,'Sweden','SE',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1205,'Switzerland','CH',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1206,'Syrian Arab Republic','SY',NULL,NULL,NULL,NULL,3,0);
INSERT INTO `r_countries` VALUES (1207,'Sao Tome and Principe','ST',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1208,'Taiwan','TW',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1209,'Tajikistan','TJ',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1210,'Tanzania, United Republic of','TZ',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1211,'Thailand','TH',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1212,'Bahamas','BS',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1213,'Gambia','GM',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1214,'Togo','TG',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1215,'Tokelau','TK',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1216,'Tonga','TO',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1217,'Trinidad and Tobago','TT',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1218,'Tunisia','TN',NULL,NULL,NULL,NULL,3,0);
INSERT INTO `r_countries` VALUES (1219,'Turkey','TR',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1220,'Turkmenistan','TM',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1221,'Turks and Caicos Islands','TC',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1222,'Tuvalu','TV',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1223,'Uganda','UG',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1224,'Ukraine','UA',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1225,'United Arab Emirates','AE',NULL,NULL,NULL,NULL,3,0);
INSERT INTO `r_countries` VALUES (1226,'United Kingdom','GB',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1227,'United States Minor Outlying Islands','UM',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1228,'United States','US',NULL,NULL,NULL,NULL,2,1);
INSERT INTO `r_countries` VALUES (1229,'Uruguay','UY',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1230,'Uzbekistan','UZ',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1231,'Vanuatu','VU',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1232,'Venezuela','VE',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1233,'Viet Nam','VN',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1234,'Virgin Islands, U.S.','VI',NULL,NULL,NULL,NULL,2,0);
INSERT INTO `r_countries` VALUES (1235,'Wallis and Futuna','WF',NULL,NULL,NULL,NULL,4,0);
INSERT INTO `r_countries` VALUES (1236,'Western Sahara','EH',NULL,NULL,NULL,NULL,3,0);
INSERT INTO `r_countries` VALUES (1237,'Yemen','YE',NULL,NULL,NULL,NULL,3,0);
INSERT INTO `r_countries` VALUES (1238,'Serbia and Montenegro','CS',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1239,'Zambia','ZM',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1240,'Zimbabwe','ZW',NULL,NULL,NULL,NULL,5,0);
INSERT INTO `r_countries` VALUES (1241,'Ã…land Islands','AX',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1242,'Serbia','RS',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1243,'Montenegro','ME',NULL,NULL,NULL,NULL,1,0);
INSERT INTO `r_countries` VALUES (1244,'Jersey','JE',NULL,NULL,NULL,NULL,99,0);
INSERT INTO `r_countries` VALUES (1245,'Guernsey','GG',NULL,NULL,NULL,NULL,99,0);
INSERT INTO `r_countries` VALUES (1246,'Isle of Man','IM',NULL,NULL,NULL,NULL,99,0);
/*!40000 ALTER TABLE `r_countries` ENABLE KEYS */;
UNLOCK TABLES;

LOCK TABLES `r_criteria` WRITE;
/*!40000 ALTER TABLE `r_criteria` DISABLE KEYS */;
INSERT INTO `r_criteria` VALUES (1,'suitable','How well does the project support our Common Good community investment priorities?',NULL,15.00,0,0,0,1541171548,1541171548);
INSERT INTO `r_criteria` VALUES (2,'systemic','How well does the project promote systemic change?',NULL,15.00,0,0,0,1541171548,1541171548);
INSERT INTO `r_criteria` VALUES (3,'doable','Overall, how clearly doable is the project?',NULL,15.00,0,0,0,1541171548,1541171548);
INSERT INTO `r_criteria` VALUES (4,'mgmt','How competent is the project team to manage the project and funds effectively?',NULL,10.00,0,0,0,1541171548,1541171548);
INSERT INTO `r_criteria` VALUES (5,'cope','How able is the project team to implement the project with less funding than requested?',NULL,5.00,0,0,0,1541171548,1541171548);
INSERT INTO `r_criteria` VALUES (6,'eval','How useful is the project\'s evaluation plan?',NULL,5.00,0,0,0,1541171548,1541171548);
INSERT INTO `r_criteria` VALUES (7,'recovery','How quickly, surely, and voluminously will the project bring funds back into our Common Good Community Dollar Pool?',NULL,10.00,0,0,0,1541171548,1541171548);
INSERT INTO `r_criteria` VALUES (8,'goodAmt','How close is the request to the ideal amount (%idealAmt)?',NULL,5.00,1,0,0,1541171548,1541171548);
INSERT INTO `r_criteria` VALUES (9,'budgetPct','What fraction of the total project budget is this funding request? (50% is ideal)',NULL,5.00,1,0,0,1541171548,1541171548);
INSERT INTO `r_criteria` VALUES (10,'committedPct','How much of the total project budget has been raised/committed so far? (half is ideal)?',NULL,5.00,1,0,0,1541171548,1541171548);
INSERT INTO `r_criteria` VALUES (11,'beginSoon','How soon does the project begin? (soon after funding is best)',NULL,2.50,1,0,0,1541171548,1541171548);
INSERT INTO `r_criteria` VALUES (12,'endSoon','How soon does the project end? (soon after funding is best)',NULL,2.50,1,0,0,1541171548,1541171548);
INSERT INTO `r_criteria` VALUES (13,'local','How local is the project?',NULL,2.50,1,0,0,1541171548,1541171548);
INSERT INTO `r_criteria` VALUES (14,'sponsor','Common Good member sponsorship of the project.',NULL,2.50,1,0,0,1541171548,1541171548);
/*!40000 ALTER TABLE `r_criteria` ENABLE KEYS */;
UNLOCK TABLES;

LOCK TABLES `r_industries` WRITE;
/*!40000 ALTER TABLE `r_industries` DISABLE KEYS */;
INSERT INTO `r_industries` VALUES (1,'Art / Culture / Science',NULL);
INSERT INTO `r_industries` VALUES (2,'Food',NULL);
INSERT INTO `r_industries` VALUES (3,'Business and Industry',NULL);
INSERT INTO `r_industries` VALUES (4,'Building / Construction / Hardware',NULL);
INSERT INTO `r_industries` VALUES (5,'Family, Community, and Society',NULL);
INSERT INTO `r_industries` VALUES (6,'Education',NULL);
INSERT INTO `r_industries` VALUES (7,'Money',NULL);
INSERT INTO `r_industries` VALUES (8,'Information and Communications',NULL);
INSERT INTO `r_industries` VALUES (9,'Sports and Recreation',NULL);
INSERT INTO `r_industries` VALUES (10,'Energy and Fuel',NULL);
INSERT INTO `r_industries` VALUES (11,'Health',NULL);
INSERT INTO `r_industries` VALUES (12,'Transportation',NULL);
INSERT INTO `r_industries` VALUES (13,'Place',NULL);
INSERT INTO `r_industries` VALUES (14,'Retail',NULL);
INSERT INTO `r_industries` VALUES (15,'Other Services',NULL);
INSERT INTO `r_industries` VALUES (16,'Visual Artists',1);
INSERT INTO `r_industries` VALUES (17,'Performance Artists',1);
INSERT INTO `r_industries` VALUES (18,'Museums and Galleries',1);
INSERT INTO `r_industries` VALUES (19,'Visual Art',1);
INSERT INTO `r_industries` VALUES (20,'Theaters and Performing Arts',1);
INSERT INTO `r_industries` VALUES (21,'Arts and Crafts Supplies',1);
INSERT INTO `r_industries` VALUES (22,'Farms',2);
INSERT INTO `r_industries` VALUES (23,'Grocers',2);
INSERT INTO `r_industries` VALUES (24,'Restaurants and Catering',2);
INSERT INTO `r_industries` VALUES (25,'Food Processing and Distribution',2);
INSERT INTO `r_industries` VALUES (26,'Engineering',3);
INSERT INTO `r_industries` VALUES (27,'Office Supplies',3);
INSERT INTO `r_industries` VALUES (28,'Computer Services',3);
INSERT INTO `r_industries` VALUES (29,'Manufacturing and Mining',3);
INSERT INTO `r_industries` VALUES (30,'Industrial Supplies and Materials',3);
INSERT INTO `r_industries` VALUES (31,'Business Services and Associations',3);
INSERT INTO `r_industries` VALUES (32,'Wholesale',3);
INSERT INTO `r_industries` VALUES (33,'Hardware and Building Supplies',4);
INSERT INTO `r_industries` VALUES (34,'Plumbing / Heating / Cooling',4);
INSERT INTO `r_industries` VALUES (35,'Electrical and Lighting',4);
INSERT INTO `r_industries` VALUES (36,'Contractors',4);
INSERT INTO `r_industries` VALUES (37,'Construction Equipment',4);
INSERT INTO `r_industries` VALUES (38,'Family and Human Services',5);
INSERT INTO `r_industries` VALUES (39,'Community Organizations',5);
INSERT INTO `r_industries` VALUES (40,'Government',5);
INSERT INTO `r_industries` VALUES (41,'Law',5);
INSERT INTO `r_industries` VALUES (42,'Educational Adventures',6);
INSERT INTO `r_industries` VALUES (43,'Schools and Colleges',6);
INSERT INTO `r_industries` VALUES (44,'Credit Unions and Banks',7);
INSERT INTO `r_industries` VALUES (45,'Insurance',7);
INSERT INTO `r_industries` VALUES (46,'Financial Services',7);
INSERT INTO `r_industries` VALUES (47,'Media',8);
INSERT INTO `r_industries` VALUES (48,'Internet',8);
INSERT INTO `r_industries` VALUES (49,'Libraries',8);
INSERT INTO `r_industries` VALUES (50,'Electronics',8);
INSERT INTO `r_industries` VALUES (51,'Hobbies',9);
INSERT INTO `r_industries` VALUES (52,'Sports Equipment',9);
INSERT INTO `r_industries` VALUES (53,'Liquor, Tobacco, and Other Recreational Drugs',9);
INSERT INTO `r_industries` VALUES (54,'Solar, Wind, and Alternative Energy',10);
INSERT INTO `r_industries` VALUES (55,'Propane and Oil',10);
INSERT INTO `r_industries` VALUES (56,'Gasoline',10);
INSERT INTO `r_industries` VALUES (57,'Gyms',11);
INSERT INTO `r_industries` VALUES (58,'Therapists',11);
INSERT INTO `r_industries` VALUES (59,'Hospitals / Clinics / Doctors / Midwives',11);
INSERT INTO `r_industries` VALUES (60,'Medical Supplies and Drugstores',11);
INSERT INTO `r_industries` VALUES (61,'Public Transit, Carpooling, and Buses',12);
INSERT INTO `r_industries` VALUES (62,'Bicycles',12);
INSERT INTO `r_industries` VALUES (63,'Cars / Automobiles',12);
INSERT INTO `r_industries` VALUES (64,'Automotive Services',12);
INSERT INTO `r_industries` VALUES (65,'Housing',13);
INSERT INTO `r_industries` VALUES (66,'Accommodations',13);
INSERT INTO `r_industries` VALUES (67,'Facilities',13);
INSERT INTO `r_industries` VALUES (68,'Real Estate',13);
INSERT INTO `r_industries` VALUES (69,'Architects',13);
INSERT INTO `r_industries` VALUES (70,'Department Stores',14);
INSERT INTO `r_industries` VALUES (71,'Bookstores',14);
INSERT INTO `r_industries` VALUES (72,'Furniture',14);
INSERT INTO `r_industries` VALUES (73,'Appliances',14);
INSERT INTO `r_industries` VALUES (74,'Clothing',14);
INSERT INTO `r_industries` VALUES (75,'Consumer Electronics',14);
INSERT INTO `r_industries` VALUES (76,'Toys and Games',14);
INSERT INTO `r_industries` VALUES (77,'Other Retail',14);
INSERT INTO `r_industries` VALUES (78,'Farm Equipment and Supplies',22);
/*!40000 ALTER TABLE `r_industries` ENABLE KEYS */;
UNLOCK TABLES;

LOCK TABLES `r_regions` WRITE;
/*!40000 ALTER TABLE `r_regions` DISABLE KEYS */;
INSERT INTO `r_regions` VALUES ('AZA',NULL,'AZ',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('CAA',NULL,'CA',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('CLN',NULL,'NC',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('CLS',NULL,'SC',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('FLA',NULL,'FL',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('GAA',NULL,'GA',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('ILA',NULL,'IL',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('INA',NULL,'IN',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('MDA',NULL,'MD',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('MIA',NULL,'MI',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('MIW','Washtenaw County','MI','^492|^48(103|104|105|107|108|109|113|115|130|158|175|176|190|191|197|198)','c/o A. Konner, 1005 Packard St., Ann Arbor, MI 48104','',0);
INSERT INTO `r_regions` VALUES ('MOA',NULL,'MO',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('MWE',NULL,'NE',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('MWI',NULL,'IA',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('MWK',NULL,'KS',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('MWM',NULL,'MN',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('MWN',NULL,'ND',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('MWS',NULL,'SD',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('NEB','Eastern Mass','MA','^02|^017|^018|^019',NULL,'',0);
INSERT INTO `r_regions` VALUES ('NEC',NULL,'CT',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('NEM',NULL,'ME',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('NEN',NULL,'NH',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('NER',NULL,'RI',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('NEV',NULL,'VT',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('NEW','Western Mass','MA',NULL,'c/o Common Good Finance, PO Box 21, Ashfield, MA 01330','461821792',1);
INSERT INTO `r_regions` VALUES ('NJA',NULL,'NJ',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('NWO',NULL,'OR',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('NWW',NULL,'WA',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('NYA',NULL,'NY',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('OHA',NULL,'OH',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('PAA',NULL,'PA',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('RMC',NULL,'CO',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('RMI',NULL,'ID',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('RMM',NULL,'MT',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('RMN',NULL,'NM',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('RMU',NULL,'UT',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('RMV',NULL,'NV',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('RMW',NULL,'WY',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('TNA',NULL,'TN',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('TXA',NULL,'TX',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('UAG',NULL,'GU',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('UAH',NULL,'HI',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('UAK',NULL,'AK',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('UAM',NULL,'MP',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('UAP',NULL,'PR',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('UAS',NULL,'AS',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('UAV',NULL,'VI',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('USB',NULL,'AL',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('USC',NULL,'DC',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('USD',NULL,'DE',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('USK',NULL,'KY',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('USL',NULL,'LA',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('USM',NULL,'MS',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('USO',NULL,'OK',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('USR',NULL,'AR',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('VRA',NULL,'VA',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('VRW',NULL,'WV',NULL,NULL,'',0);
INSERT INTO `r_regions` VALUES ('WIA',NULL,'WI',NULL,NULL,'',0);
/*!40000 ALTER TABLE `r_regions` ENABLE KEYS */;
UNLOCK TABLES;

LOCK TABLES `r_states` WRITE;
/*!40000 ALTER TABLE `r_states` DISABLE KEYS */;
INSERT INTO `r_states` VALUES (1000,'Alabama','AL',1228);
INSERT INTO `r_states` VALUES (1001,'Alaska','AK',1228);
INSERT INTO `r_states` VALUES (1002,'Arizona','AZ',1228);
INSERT INTO `r_states` VALUES (1003,'Arkansas','AR',1228);
INSERT INTO `r_states` VALUES (1004,'California','CA',1228);
INSERT INTO `r_states` VALUES (1005,'Colorado','CO',1228);
INSERT INTO `r_states` VALUES (1006,'Connecticut','CT',1228);
INSERT INTO `r_states` VALUES (1007,'Delaware','DE',1228);
INSERT INTO `r_states` VALUES (1008,'Florida','FL',1228);
INSERT INTO `r_states` VALUES (1009,'Georgia','GA',1228);
INSERT INTO `r_states` VALUES (1010,'Hawaii','HI',1228);
INSERT INTO `r_states` VALUES (1011,'Idaho','ID',1228);
INSERT INTO `r_states` VALUES (1012,'Illinois','IL',1228);
INSERT INTO `r_states` VALUES (1013,'Indiana','IN',1228);
INSERT INTO `r_states` VALUES (1014,'Iowa','IA',1228);
INSERT INTO `r_states` VALUES (1015,'Kansas','KS',1228);
INSERT INTO `r_states` VALUES (1016,'Kentucky','KY',1228);
INSERT INTO `r_states` VALUES (1017,'Louisiana','LA',1228);
INSERT INTO `r_states` VALUES (1018,'Maine','ME',1228);
INSERT INTO `r_states` VALUES (1019,'Maryland','MD',1228);
INSERT INTO `r_states` VALUES (1020,'Massachusetts','MA',1228);
INSERT INTO `r_states` VALUES (1021,'Michigan','MI',1228);
INSERT INTO `r_states` VALUES (1022,'Minnesota','MN',1228);
INSERT INTO `r_states` VALUES (1023,'Mississippi','MS',1228);
INSERT INTO `r_states` VALUES (1024,'Missouri','MO',1228);
INSERT INTO `r_states` VALUES (1025,'Montana','MT',1228);
INSERT INTO `r_states` VALUES (1026,'Nebraska','NE',1228);
INSERT INTO `r_states` VALUES (1027,'Nevada','NV',1228);
INSERT INTO `r_states` VALUES (1028,'New Hampshire','NH',1228);
INSERT INTO `r_states` VALUES (1029,'New Jersey','NJ',1228);
INSERT INTO `r_states` VALUES (1030,'New Mexico','NM',1228);
INSERT INTO `r_states` VALUES (1031,'New York','NY',1228);
INSERT INTO `r_states` VALUES (1032,'North Carolina','NC',1228);
INSERT INTO `r_states` VALUES (1033,'North Dakota','ND',1228);
INSERT INTO `r_states` VALUES (1034,'Ohio','OH',1228);
INSERT INTO `r_states` VALUES (1035,'Oklahoma','OK',1228);
INSERT INTO `r_states` VALUES (1036,'Oregon','OR',1228);
INSERT INTO `r_states` VALUES (1037,'Pennsylvania','PA',1228);
INSERT INTO `r_states` VALUES (1038,'Rhode Island','RI',1228);
INSERT INTO `r_states` VALUES (1039,'South Carolina','SC',1228);
INSERT INTO `r_states` VALUES (1040,'South Dakota','SD',1228);
INSERT INTO `r_states` VALUES (1041,'Tennessee','TN',1228);
INSERT INTO `r_states` VALUES (1042,'Texas','TX',1228);
INSERT INTO `r_states` VALUES (1043,'Utah','UT',1228);
INSERT INTO `r_states` VALUES (1044,'Vermont','VT',1228);
INSERT INTO `r_states` VALUES (1045,'Virginia','VA',1228);
INSERT INTO `r_states` VALUES (1046,'Washington','WA',1228);
INSERT INTO `r_states` VALUES (1047,'West Virginia','WV',1228);
INSERT INTO `r_states` VALUES (1048,'Wisconsin','WI',1228);
INSERT INTO `r_states` VALUES (1049,'Wyoming','WY',1228);
INSERT INTO `r_states` VALUES (1050,'District of Columbia','DC',1228);
INSERT INTO `r_states` VALUES (1052,'American Samoa','AS',1228);
INSERT INTO `r_states` VALUES (1053,'Guam','GU',1228);
INSERT INTO `r_states` VALUES (1055,'Northern Mariana Islands','MP',1228);
INSERT INTO `r_states` VALUES (1056,'Puerto Rico','PR',1228);
INSERT INTO `r_states` VALUES (1057,'Virgin Islands','VI',1228);
INSERT INTO `r_states` VALUES (1058,'United States Minor Outlying Islands','UM',1228);
INSERT INTO `r_states` VALUES (1059,'Armed Forces Europe','AE',1228);
INSERT INTO `r_states` VALUES (1060,'Armed Forces Americas','AA',1228);
INSERT INTO `r_states` VALUES (1061,'Armed Forces Pacific','AP',1228);
INSERT INTO `r_states` VALUES (2515,'Cher','18',1076);
/*!40000 ALTER TABLE `r_states` ENABLE KEYS */;
UNLOCK TABLES;

LOCK TABLES `r_transit` WRITE;
/*!40000 ALTER TABLE `r_transit` DISABLE KEYS */;
INSERT INTO `r_transit` VALUES (93,'12');
INSERT INTO `r_transit` VALUES (59,'AK');
INSERT INTO `r_transit` VALUES (61,'AL');
INSERT INTO `r_transit` VALUES (29,'Albany, NY');
INSERT INTO `r_transit` VALUES (81,'AR');
INSERT INTO `r_transit` VALUES (59,'AS');
INSERT INTO `r_transit` VALUES (91,'AZ');
INSERT INTO `r_transit` VALUES (7,'Baltimore, MD');
INSERT INTO `r_transit` VALUES (5,'Boston, MA');
INSERT INTO `r_transit` VALUES (10,'Buffalo, NY');
INSERT INTO `r_transit` VALUES (90,'CA');
INSERT INTO `r_transit` VALUES (47,'Cedar Rapids, IA');
INSERT INTO `r_transit` VALUES (2,'Chicago, IL');
INSERT INTO `r_transit` VALUES (13,'Cincinnati, OH');
INSERT INTO `r_transit` VALUES (6,'Cleveland, OH');
INSERT INTO `r_transit` VALUES (82,'CO');
INSERT INTO `r_transit` VALUES (25,'Columbus, OH');
INSERT INTO `r_transit` VALUES (51,'CT');
INSERT INTO `r_transit` VALUES (32,'Dallas, TX');
INSERT INTO `r_transit` VALUES (62,'DE');
INSERT INTO `r_transit` VALUES (23,'Denver, CO');
INSERT INTO `r_transit` VALUES (33,'Des Moines, IA');
INSERT INTO `r_transit` VALUES (9,'Detroit, MI');
INSERT INTO `r_transit` VALUES (45,'Dubuque, IA');
INSERT INTO `r_transit` VALUES (63,'FL');
INSERT INTO `r_transit` VALUES (37,'Fort Worth, TX');
INSERT INTO `r_transit` VALUES (64,'GA');
INSERT INTO `r_transit` VALUES (46,'Galveston, TX');
INSERT INTO `r_transit` VALUES (59,'GU');
INSERT INTO `r_transit` VALUES (59,'HI');
INSERT INTO `r_transit` VALUES (35,'Houston, TX');
INSERT INTO `r_transit` VALUES (72,'IA');
INSERT INTO `r_transit` VALUES (92,'ID');
INSERT INTO `r_transit` VALUES (70,'IL');
INSERT INTO `r_transit` VALUES (71,'IN');
INSERT INTO `r_transit` VALUES (20,'Indianapolis, IN');
INSERT INTO `r_transit` VALUES (18,'Kansas City, MO');
INSERT INTO `r_transit` VALUES (83,'KS');
INSERT INTO `r_transit` VALUES (73,'KY');
INSERT INTO `r_transit` VALUES (84,'LA');
INSERT INTO `r_transit` VALUES (43,'Lincoln, NE');
INSERT INTO `r_transit` VALUES (16,'Los Angeles, CA');
INSERT INTO `r_transit` VALUES (21,'Louisville, KY');
INSERT INTO `r_transit` VALUES (53,'MA');
INSERT INTO `r_transit` VALUES (52,'ME');
INSERT INTO `r_transit` VALUES (26,'Memphis, TN');
INSERT INTO `r_transit` VALUES (74,'MI');
INSERT INTO `r_transit` VALUES (12,'Milwaukee, WI');
INSERT INTO `r_transit` VALUES (17,'Minneapolis, MN');
INSERT INTO `r_transit` VALUES (75,'MN');
INSERT INTO `r_transit` VALUES (80,'MO');
INSERT INTO `r_transit` VALUES (85,'MS');
INSERT INTO `r_transit` VALUES (49,'Muskogee, OK');
INSERT INTO `r_transit` VALUES (65,'MY');
INSERT INTO `r_transit` VALUES (66,'NC');
INSERT INTO `r_transit` VALUES (77,'ND');
INSERT INTO `r_transit` VALUES (76,'NE');
INSERT INTO `r_transit` VALUES (14,'New Orleans, LA');
INSERT INTO `r_transit` VALUES (1,'New York, NY');
INSERT INTO `r_transit` VALUES (54,'NH');
INSERT INTO `r_transit` VALUES (55,'NJ');
INSERT INTO `r_transit` VALUES (95,'NM');
INSERT INTO `r_transit` VALUES (94,'NV');
INSERT INTO `r_transit` VALUES (50,'NY');
INSERT INTO `r_transit` VALUES (56,'OH');
INSERT INTO `r_transit` VALUES (86,'OK');
INSERT INTO `r_transit` VALUES (39,'Oklahoma City, OK');
INSERT INTO `r_transit` VALUES (27,'Omaha, NE');
INSERT INTO `r_transit` VALUES (96,'OR');
INSERT INTO `r_transit` VALUES (60,'PA');
INSERT INTO `r_transit` VALUES (3,'Philadelphia, PA');
INSERT INTO `r_transit` VALUES (8,'Pittsburgh, PA');
INSERT INTO `r_transit` VALUES (24,'Portland, OR');
INSERT INTO `r_transit` VALUES (59,'PR');
INSERT INTO `r_transit` VALUES (42,'Pueblo, CO');
INSERT INTO `r_transit` VALUES (57,'RI');
INSERT INTO `r_transit` VALUES (31,'Salt Lake City, UT');
INSERT INTO `r_transit` VALUES (30,'San Antonio, TX');
INSERT INTO `r_transit` VALUES (11,'San Francisco, CA');
INSERT INTO `r_transit` VALUES (38,'Savannah, GA');
INSERT INTO `r_transit` VALUES (67,'SC');
INSERT INTO `r_transit` VALUES (78,'SD');
INSERT INTO `r_transit` VALUES (19,'Seattle, WA');
INSERT INTO `r_transit` VALUES (41,'Sioux City, IA');
INSERT INTO `r_transit` VALUES (28,'Spokane, WA');
INSERT INTO `r_transit` VALUES (36,'St. Joseph, MO');
INSERT INTO `r_transit` VALUES (4,'St. Louis, MO');
INSERT INTO `r_transit` VALUES (22,'St. Paul, MN');
INSERT INTO `r_transit` VALUES (34,'Tacoma, WA');
INSERT INTO `r_transit` VALUES (87,'TN');
INSERT INTO `r_transit` VALUES (44,'Topeka, KS');
INSERT INTO `r_transit` VALUES (88,'TX');
INSERT INTO `r_transit` VALUES (97,'UT');
INSERT INTO `r_transit` VALUES (68,'VA');
INSERT INTO `r_transit` VALUES (59,'VI');
INSERT INTO `r_transit` VALUES (58,'VT');
INSERT INTO `r_transit` VALUES (98,'WA');
INSERT INTO `r_transit` VALUES (48,'Waco, TX');
INSERT INTO `r_transit` VALUES (15,'Washington D.C.');
INSERT INTO `r_transit` VALUES (79,'WI');
INSERT INTO `r_transit` VALUES (40,'Wichita, KS');
INSERT INTO `r_transit` VALUES (69,'WV');
INSERT INTO `r_transit` VALUES (99,'WY');
/*!40000 ALTER TABLE `r_transit` ENABLE KEYS */;
UNLOCK TABLES;

LOCK TABLES `registry` WRITE;
/*!40000 ALTER TABLE `registry` DISABLE KEYS */;
INSERT INTO `registry` VALUES ('AccessDeniedTestCase','class','modules/system/system.test','system',0);
INSERT INTO `registry` VALUES ('AdminMetaTagTestCase','class','modules/system/system.test','system',0);
INSERT INTO `registry` VALUES ('Archive_Tar','class','modules/system/system.tar.inc','system',0);
INSERT INTO `registry` VALUES ('ArchiverInterface','interface','includes/archiver.inc','',0);
INSERT INTO `registry` VALUES ('ArchiverTar','class','modules/system/system.archiver.inc','system',0);
INSERT INTO `registry` VALUES ('ArchiverZip','class','modules/system/system.archiver.inc','system',0);
INSERT INTO `registry` VALUES ('BatchMemoryQueue','class','includes/batch.queue.inc','',0);
INSERT INTO `registry` VALUES ('BatchQueue','class','includes/batch.queue.inc','',0);
INSERT INTO `registry` VALUES ('BlockAdminThemeTestCase','class','modules/block/block.test','block',-5);
INSERT INTO `registry` VALUES ('BlockCacheTestCase','class','modules/block/block.test','block',-5);
INSERT INTO `registry` VALUES ('BlockHashTestCase','class','modules/block/block.test','block',-5);
INSERT INTO `registry` VALUES ('BlockHiddenRegionTestCase','class','modules/block/block.test','block',-5);
INSERT INTO `registry` VALUES ('BlockHTMLIdTestCase','class','modules/block/block.test','block',-5);
INSERT INTO `registry` VALUES ('BlockInvalidRegionTestCase','class','modules/block/block.test','block',-5);
INSERT INTO `registry` VALUES ('BlockTemplateSuggestionsUnitTest','class','modules/block/block.test','block',-5);
INSERT INTO `registry` VALUES ('BlockTestCase','class','modules/block/block.test','block',-5);
INSERT INTO `registry` VALUES ('BlockViewModuleDeltaAlterWebTest','class','modules/block/block.test','block',-5);
INSERT INTO `registry` VALUES ('call','class','rcredits/rsms/rsms-call.inc','rsms',0);
INSERT INTO `registry` VALUES ('CronQueueTestCase','class','modules/system/system.test','system',0);
INSERT INTO `registry` VALUES ('CronRunTestCase','class','modules/system/system.test','system',0);
INSERT INTO `registry` VALUES ('Database','class','includes/database/database.inc','',0);
INSERT INTO `registry` VALUES ('DatabaseCondition','class','includes/database/query.inc','',0);
INSERT INTO `registry` VALUES ('DatabaseConnection','class','includes/database/database.inc','',0);
INSERT INTO `registry` VALUES ('DatabaseConnection_mysql','class','includes/database/mysql/database.inc','',0);
INSERT INTO `registry` VALUES ('DatabaseConnection_pgsql','class','includes/database/pgsql/database.inc','',0);
INSERT INTO `registry` VALUES ('DatabaseConnection_sqlite','class','includes/database/sqlite/database.inc','',0);
INSERT INTO `registry` VALUES ('DatabaseConnectionNotDefinedException','class','includes/database/database.inc','',0);
INSERT INTO `registry` VALUES ('DatabaseDriverNotSpecifiedException','class','includes/database/database.inc','',0);
INSERT INTO `registry` VALUES ('DatabaseLog','class','includes/database/log.inc','',0);
INSERT INTO `registry` VALUES ('DatabaseSchema','class','includes/database/schema.inc','',0);
INSERT INTO `registry` VALUES ('DatabaseSchema_mysql','class','includes/database/mysql/schema.inc','',0);
INSERT INTO `registry` VALUES ('DatabaseSchema_pgsql','class','includes/database/pgsql/schema.inc','',0);
INSERT INTO `registry` VALUES ('DatabaseSchema_sqlite','class','includes/database/sqlite/schema.inc','',0);
INSERT INTO `registry` VALUES ('DatabaseSchemaObjectDoesNotExistException','class','includes/database/schema.inc','',0);
INSERT INTO `registry` VALUES ('DatabaseSchemaObjectExistsException','class','includes/database/schema.inc','',0);
INSERT INTO `registry` VALUES ('DatabaseStatement_sqlite','class','includes/database/sqlite/database.inc','',0);
INSERT INTO `registry` VALUES ('DatabaseStatementBase','class','includes/database/database.inc','',0);
INSERT INTO `registry` VALUES ('DatabaseStatementEmpty','class','includes/database/database.inc','',0);
INSERT INTO `registry` VALUES ('DatabaseStatementInterface','interface','includes/database/database.inc','',0);
INSERT INTO `registry` VALUES ('DatabaseStatementPrefetch','class','includes/database/prefetch.inc','',0);
INSERT INTO `registry` VALUES ('DatabaseTaskException','class','includes/install.inc','',0);
INSERT INTO `registry` VALUES ('DatabaseTasks','class','includes/install.inc','',0);
INSERT INTO `registry` VALUES ('DatabaseTasks_mysql','class','includes/database/mysql/install.inc','',0);
INSERT INTO `registry` VALUES ('DatabaseTasks_pgsql','class','includes/database/pgsql/install.inc','',0);
INSERT INTO `registry` VALUES ('DatabaseTasks_sqlite','class','includes/database/sqlite/install.inc','',0);
INSERT INTO `registry` VALUES ('DatabaseTransaction','class','includes/database/database.inc','',0);
INSERT INTO `registry` VALUES ('DatabaseTransactionCommitFailedException','class','includes/database/database.inc','',0);
INSERT INTO `registry` VALUES ('DatabaseTransactionExplicitCommitNotAllowedException','class','includes/database/database.inc','',0);
INSERT INTO `registry` VALUES ('DatabaseTransactionNameNonUniqueException','class','includes/database/database.inc','',0);
INSERT INTO `registry` VALUES ('DatabaseTransactionNoActiveException','class','includes/database/database.inc','',0);
INSERT INTO `registry` VALUES ('DatabaseTransactionOutOfOrderException','class','includes/database/database.inc','',0);
INSERT INTO `registry` VALUES ('DateTimeFunctionalTest','class','modules/system/system.test','system',0);
INSERT INTO `registry` VALUES ('DefaultMailSystem','class','modules/system/system.mail.inc','system',0);
INSERT INTO `registry` VALUES ('DeleteQuery','class','includes/database/query.inc','',0);
INSERT INTO `registry` VALUES ('DeleteQuery_sqlite','class','includes/database/sqlite/query.inc','',0);
INSERT INTO `registry` VALUES ('DevelMailLog','class','sites/all/modules/devel/devel.mail.inc','devel',0);
INSERT INTO `registry` VALUES ('DevelMailTest','class','sites/all/modules/devel/devel.test','devel',0);
INSERT INTO `registry` VALUES ('DrupalCacheArray','class','includes/bootstrap.inc','',0);
INSERT INTO `registry` VALUES ('DrupalCacheInterface','interface','includes/cache.inc','',0);
INSERT INTO `registry` VALUES ('DrupalDatabaseCache','class','includes/cache.inc','',0);
INSERT INTO `registry` VALUES ('DrupalDefaultEntityController','class','includes/entity.inc','',0);
INSERT INTO `registry` VALUES ('DrupalEntityControllerInterface','interface','includes/entity.inc','',0);
INSERT INTO `registry` VALUES ('DrupalFakeCache','class','includes/cache-install.inc','',0);
INSERT INTO `registry` VALUES ('DrupalLocalStreamWrapper','class','includes/stream_wrappers.inc','',0);
INSERT INTO `registry` VALUES ('DrupalPrivateStreamWrapper','class','includes/stream_wrappers.inc','',0);
INSERT INTO `registry` VALUES ('DrupalPublicStreamWrapper','class','includes/stream_wrappers.inc','',0);
INSERT INTO `registry` VALUES ('DrupalQueue','class','modules/system/system.queue.inc','system',0);
INSERT INTO `registry` VALUES ('DrupalQueueInterface','interface','modules/system/system.queue.inc','system',0);
INSERT INTO `registry` VALUES ('DrupalReliableQueueInterface','interface','modules/system/system.queue.inc','system',0);
INSERT INTO `registry` VALUES ('DrupalStreamWrapperInterface','interface','includes/stream_wrappers.inc','',0);
INSERT INTO `registry` VALUES ('DrupalTemporaryStreamWrapper','class','includes/stream_wrappers.inc','',0);
INSERT INTO `registry` VALUES ('DrupalUpdateException','class','includes/update.inc','',0);
INSERT INTO `registry` VALUES ('DrupalUpdaterInterface','interface','includes/updater.inc','',0);
INSERT INTO `registry` VALUES ('EnableDisableTestCase','class','modules/system/system.test','system',0);
INSERT INTO `registry` VALUES ('EntityFieldQuery','class','includes/entity.inc','',0);
INSERT INTO `registry` VALUES ('EntityFieldQueryException','class','includes/entity.inc','',0);
INSERT INTO `registry` VALUES ('EntityMalformedException','class','includes/entity.inc','',0);
INSERT INTO `registry` VALUES ('EntityPropertiesTestCase','class','modules/field/tests/field.test','field',0);
INSERT INTO `registry` VALUES ('FieldAttachOtherTestCase','class','modules/field/tests/field.test','field',0);
INSERT INTO `registry` VALUES ('FieldAttachStorageTestCase','class','modules/field/tests/field.test','field',0);
INSERT INTO `registry` VALUES ('FieldAttachTestCase','class','modules/field/tests/field.test','field',0);
INSERT INTO `registry` VALUES ('FieldBulkDeleteTestCase','class','modules/field/tests/field.test','field',0);
INSERT INTO `registry` VALUES ('FieldCrudTestCase','class','modules/field/tests/field.test','field',0);
INSERT INTO `registry` VALUES ('FieldDisplayAPITestCase','class','modules/field/tests/field.test','field',0);
INSERT INTO `registry` VALUES ('FieldException','class','modules/field/field.module','field',0);
INSERT INTO `registry` VALUES ('FieldFormTestCase','class','modules/field/tests/field.test','field',0);
INSERT INTO `registry` VALUES ('FieldInfo','class','modules/field/field.info.class.inc','field',0);
INSERT INTO `registry` VALUES ('FieldInfoTestCase','class','modules/field/tests/field.test','field',0);
INSERT INTO `registry` VALUES ('FieldInstanceCrudTestCase','class','modules/field/tests/field.test','field',0);
INSERT INTO `registry` VALUES ('FieldsOverlapException','class','includes/database/database.inc','',0);
INSERT INTO `registry` VALUES ('FieldSqlStorageTestCase','class','modules/field/modules/field_sql_storage/field_sql_storage.test','field_sql_storage',0);
INSERT INTO `registry` VALUES ('FieldTestCase','class','modules/field/tests/field.test','field',0);
INSERT INTO `registry` VALUES ('FieldTranslationsTestCase','class','modules/field/tests/field.test','field',0);
INSERT INTO `registry` VALUES ('FieldUpdateForbiddenException','class','modules/field/field.module','field',0);
INSERT INTO `registry` VALUES ('FieldValidationException','class','modules/field/field.attach.inc','field',0);
INSERT INTO `registry` VALUES ('FileFieldDisplayTestCase','class','modules/file/tests/file.test','file',0);
INSERT INTO `registry` VALUES ('FileFieldPathTestCase','class','modules/file/tests/file.test','file',0);
INSERT INTO `registry` VALUES ('FileFieldRevisionTestCase','class','modules/file/tests/file.test','file',0);
INSERT INTO `registry` VALUES ('FileFieldTestCase','class','modules/file/tests/file.test','file',0);
INSERT INTO `registry` VALUES ('FileFieldValidateTestCase','class','modules/file/tests/file.test','file',0);
INSERT INTO `registry` VALUES ('FileFieldWidgetTestCase','class','modules/file/tests/file.test','file',0);
INSERT INTO `registry` VALUES ('FileManagedFileElementTestCase','class','modules/file/tests/file.test','file',0);
INSERT INTO `registry` VALUES ('FilePrivateTestCase','class','modules/file/tests/file.test','file',0);
INSERT INTO `registry` VALUES ('FileTaxonomyTermTestCase','class','modules/file/tests/file.test','file',0);
INSERT INTO `registry` VALUES ('FileTokenReplaceTestCase','class','modules/file/tests/file.test','file',0);
INSERT INTO `registry` VALUES ('FileTransfer','class','includes/filetransfer/filetransfer.inc','',0);
INSERT INTO `registry` VALUES ('FileTransferChmodInterface','interface','includes/filetransfer/filetransfer.inc','',0);
INSERT INTO `registry` VALUES ('FileTransferException','class','includes/filetransfer/filetransfer.inc','',0);
INSERT INTO `registry` VALUES ('FileTransferFTP','class','includes/filetransfer/ftp.inc','',0);
INSERT INTO `registry` VALUES ('FileTransferFTPExtension','class','includes/filetransfer/ftp.inc','',0);
INSERT INTO `registry` VALUES ('FileTransferLocal','class','includes/filetransfer/local.inc','',0);
INSERT INTO `registry` VALUES ('FileTransferSSH','class','includes/filetransfer/ssh.inc','',0);
INSERT INTO `registry` VALUES ('FilterAdminTestCase','class','modules/filter/filter.test','filter',0);
INSERT INTO `registry` VALUES ('FilterCRUDTestCase','class','modules/filter/filter.test','filter',0);
INSERT INTO `registry` VALUES ('FilterDefaultFormatTestCase','class','modules/filter/filter.test','filter',0);
INSERT INTO `registry` VALUES ('FilterFormatAccessTestCase','class','modules/filter/filter.test','filter',0);
INSERT INTO `registry` VALUES ('FilterHooksTestCase','class','modules/filter/filter.test','filter',0);
INSERT INTO `registry` VALUES ('FilterNoFormatTestCase','class','modules/filter/filter.test','filter',0);
INSERT INTO `registry` VALUES ('FilterSecurityTestCase','class','modules/filter/filter.test','filter',0);
INSERT INTO `registry` VALUES ('FilterSettingsTestCase','class','modules/filter/filter.test','filter',0);
INSERT INTO `registry` VALUES ('FilterUnitTestCase','class','modules/filter/filter.test','filter',0);
INSERT INTO `registry` VALUES ('FloodFunctionalTest','class','modules/system/system.test','system',0);
INSERT INTO `registry` VALUES ('FrontPageTestCase','class','modules/system/system.test','system',0);
INSERT INTO `registry` VALUES ('HookRequirementsTestCase','class','modules/system/system.test','system',0);
INSERT INTO `registry` VALUES ('InfoFileParserTestCase','class','modules/system/system.test','system',0);
INSERT INTO `registry` VALUES ('InsertQuery','class','includes/database/query.inc','',0);
INSERT INTO `registry` VALUES ('InsertQuery_mysql','class','includes/database/mysql/query.inc','',0);
INSERT INTO `registry` VALUES ('InsertQuery_pgsql','class','includes/database/pgsql/query.inc','',0);
INSERT INTO `registry` VALUES ('InsertQuery_sqlite','class','includes/database/sqlite/query.inc','',0);
INSERT INTO `registry` VALUES ('InvalidMergeQueryException','class','includes/database/database.inc','',0);
INSERT INTO `registry` VALUES ('IPAddressBlockingTestCase','class','modules/system/system.test','system',0);
INSERT INTO `registry` VALUES ('MailSystemInterface','interface','includes/mail.inc','',0);
INSERT INTO `registry` VALUES ('MemoryQueue','class','modules/system/system.queue.inc','system',0);
INSERT INTO `registry` VALUES ('MergeQuery','class','includes/database/query.inc','',0);
INSERT INTO `registry` VALUES ('ModuleDependencyTestCase','class','modules/system/system.test','system',0);
INSERT INTO `registry` VALUES ('ModuleRequiredTestCase','class','modules/system/system.test','system',0);
INSERT INTO `registry` VALUES ('ModuleTestCase','class','modules/system/system.test','system',0);
INSERT INTO `registry` VALUES ('ModuleUpdater','class','modules/system/system.updater.inc','system',0);
INSERT INTO `registry` VALUES ('ModuleVersionTestCase','class','modules/system/system.test','system',0);
INSERT INTO `registry` VALUES ('MultiStepNodeFormBasicOptionsTest','class','modules/node/node.test','node',0);
INSERT INTO `registry` VALUES ('NewDefaultThemeBlocks','class','modules/block/block.test','block',-5);
INSERT INTO `registry` VALUES ('NodeAccessBaseTableTestCase','class','modules/node/node.test','node',0);
INSERT INTO `registry` VALUES ('NodeAccessFieldTestCase','class','modules/node/node.test','node',0);
INSERT INTO `registry` VALUES ('NodeAccessPagerTestCase','class','modules/node/node.test','node',0);
INSERT INTO `registry` VALUES ('NodeAccessRebuildTestCase','class','modules/node/node.test','node',0);
INSERT INTO `registry` VALUES ('NodeAccessRecordsTestCase','class','modules/node/node.test','node',0);
INSERT INTO `registry` VALUES ('NodeAccessTestCase','class','modules/node/node.test','node',0);
INSERT INTO `registry` VALUES ('NodeAdminTestCase','class','modules/node/node.test','node',0);
INSERT INTO `registry` VALUES ('NodeBlockFunctionalTest','class','modules/node/node.test','node',0);
INSERT INTO `registry` VALUES ('NodeBlockTestCase','class','modules/node/node.test','node',0);
INSERT INTO `registry` VALUES ('NodeBuildContent','class','modules/node/node.test','node',0);
INSERT INTO `registry` VALUES ('NodeController','class','modules/node/node.module','node',0);
INSERT INTO `registry` VALUES ('NodeCreationTestCase','class','modules/node/node.test','node',0);
INSERT INTO `registry` VALUES ('NodeEntityFieldQueryAlter','class','modules/node/node.test','node',0);
INSERT INTO `registry` VALUES ('NodeEntityViewModeAlterTest','class','modules/node/node.test','node',0);
INSERT INTO `registry` VALUES ('NodeFeedTestCase','class','modules/node/node.test','node',0);
INSERT INTO `registry` VALUES ('NodeLoadHooksTestCase','class','modules/node/node.test','node',0);
INSERT INTO `registry` VALUES ('NodeLoadMultipleTestCase','class','modules/node/node.test','node',0);
INSERT INTO `registry` VALUES ('NodePageCacheTest','class','modules/node/node.test','node',0);
INSERT INTO `registry` VALUES ('NodePostSettingsTestCase','class','modules/node/node.test','node',0);
INSERT INTO `registry` VALUES ('NodeQueryAlter','class','modules/node/node.test','node',0);
INSERT INTO `registry` VALUES ('NodeRevisionPermissionsTestCase','class','modules/node/node.test','node',0);
INSERT INTO `registry` VALUES ('NodeRevisionsTestCase','class','modules/node/node.test','node',0);
INSERT INTO `registry` VALUES ('NodeRSSContentTestCase','class','modules/node/node.test','node',0);
INSERT INTO `registry` VALUES ('NodeSaveTestCase','class','modules/node/node.test','node',0);
INSERT INTO `registry` VALUES ('NodeTitleTestCase','class','modules/node/node.test','node',0);
INSERT INTO `registry` VALUES ('NodeTitleXSSTestCase','class','modules/node/node.test','node',0);
INSERT INTO `registry` VALUES ('NodeTokenReplaceTestCase','class','modules/node/node.test','node',0);
INSERT INTO `registry` VALUES ('NodeTypePersistenceTestCase','class','modules/node/node.test','node',0);
INSERT INTO `registry` VALUES ('NodeTypeTestCase','class','modules/node/node.test','node',0);
INSERT INTO `registry` VALUES ('NodeWebTestCase','class','modules/node/node.test','node',0);
INSERT INTO `registry` VALUES ('NoFieldsException','class','includes/database/database.inc','',0);
INSERT INTO `registry` VALUES ('NonDefaultBlockAdmin','class','modules/block/block.test','block',-5);
INSERT INTO `registry` VALUES ('PageEditTestCase','class','modules/node/node.test','node',0);
INSERT INTO `registry` VALUES ('PageNotFoundTestCase','class','modules/system/system.test','system',0);
INSERT INTO `registry` VALUES ('PagePreviewTestCase','class','modules/node/node.test','node',0);
INSERT INTO `registry` VALUES ('PagerDefault','class','includes/pager.inc','',0);
INSERT INTO `registry` VALUES ('PageTitleFiltering','class','modules/system/system.test','system',0);
INSERT INTO `registry` VALUES ('PageViewTestCase','class','modules/node/node.test','node',0);
INSERT INTO `registry` VALUES ('Query','class','includes/database/query.inc','',0);
INSERT INTO `registry` VALUES ('QueryAlterableInterface','interface','includes/database/query.inc','',0);
INSERT INTO `registry` VALUES ('QueryConditionInterface','interface','includes/database/query.inc','',0);
INSERT INTO `registry` VALUES ('QueryExtendableInterface','interface','includes/database/select.inc','',0);
INSERT INTO `registry` VALUES ('QueryPlaceholderInterface','interface','includes/database/query.inc','',0);
INSERT INTO `registry` VALUES ('QueueTestCase','class','modules/system/system.test','system',0);
INSERT INTO `registry` VALUES ('RetrieveFileTestCase','class','modules/system/system.test','system',0);
INSERT INTO `registry` VALUES ('rsmartadhoc','class','rcredits/rsmart/tests/adhoc.test','rsmart',0);
INSERT INTO `registry` VALUES ('rsmartChange','class','rcredits/rsmart/tests/Change.test','rsmart',0);
INSERT INTO `registry` VALUES ('rsmartIdentifyQR','class','rcredits/rsmart/tests/IdentifyQR.test','rsmart',0);
INSERT INTO `registry` VALUES ('rsmartInsufficient','class','rcredits/rsmart/tests/Insufficient.test','rsmart',0);
INSERT INTO `registry` VALUES ('rsmartStartup','class','rcredits/rsmart/tests/Startup.test','rsmart',0);
INSERT INTO `registry` VALUES ('rsmartTransact','class','rcredits/rsmart/tests/Transact.test','rsmart',0);
INSERT INTO `registry` VALUES ('rsmartUndo','class','rcredits/rsmart/tests/Undo.test','rsmart',0);
INSERT INTO `registry` VALUES ('rsmartUndoAttack','class','rcredits/rsmart/tests/UndoAttack.test','rsmart',0);
INSERT INTO `registry` VALUES ('rsmartUndoCompleted','class','rcredits/rsmart/tests/UndoCompleted.test','rsmart',0);
INSERT INTO `registry` VALUES ('rsmartUndoPending','class','rcredits/rsmart/tests/UndoPending.test','rsmart',0);
INSERT INTO `registry` VALUES ('rsmartUnilateral','class','rcredits/rsmart/tests/Unilateral.test','rsmart',0);
INSERT INTO `registry` VALUES ('rsmsAbbreviationsWork','class','rcredits/rsms/tests/AbbreviationsWork.test','rsms',0);
INSERT INTO `registry` VALUES ('rSMSexception','class','rcredits/rsms/rsms-call.inc','rsms',0);
INSERT INTO `registry` VALUES ('rsmsExchangeForCash','class','rcredits/rsms/tests/ExchangeForCash.test','rsms',0);
INSERT INTO `registry` VALUES ('rsmsGetHelp','class','rcredits/rsms/tests/GetHelp.test','rsms',0);
INSERT INTO `registry` VALUES ('rsmsGetInformation','class','rcredits/rsms/tests/GetInformation.test','rsms',0);
INSERT INTO `registry` VALUES ('rsmsOfferToExchangeUSDollarsForRCredits','class','rcredits/rsms/tests/OfferToExchangeUSDollarsForRCredits.test','rsms',0);
INSERT INTO `registry` VALUES ('rsmsOpenAnAccountForTheCaller','class','rcredits/rsms/tests/OpenAnAccountForTheCaller.test','rsms',0);
INSERT INTO `registry` VALUES ('rsmsTransact','class','rcredits/rsms/tests/Transact.test','rsms',0);
INSERT INTO `registry` VALUES ('rsmsUndo','class','rcredits/rsms/tests/Undo.test','rsms',0);
INSERT INTO `registry` VALUES ('rwebmulti','class','rcredits/rweb/tests/multi.test','rweb',0);
INSERT INTO `registry` VALUES ('rwebRelations','class','rcredits/rweb/tests/Relations.test','rweb',0);
INSERT INTO `registry` VALUES ('rwebSignup','class','rcredits/rweb/tests/Signup.test','rweb',0);
INSERT INTO `registry` VALUES ('rwebSummary','class','rcredits/rweb/tests/Summary.test','rweb',0);
INSERT INTO `registry` VALUES ('rwebTransact','class','rcredits/rweb/tests/Transact.test','rweb',0);
INSERT INTO `registry` VALUES ('rwebTransactions','class','rcredits/rweb/tests/Transactions.test','rweb',0);
INSERT INTO `registry` VALUES ('SchemaCache','class','includes/bootstrap.inc','',0);
INSERT INTO `registry` VALUES ('SelectQuery','class','includes/database/select.inc','',0);
INSERT INTO `registry` VALUES ('SelectQuery_pgsql','class','includes/database/pgsql/select.inc','',0);
INSERT INTO `registry` VALUES ('SelectQuery_sqlite','class','includes/database/sqlite/select.inc','',0);
INSERT INTO `registry` VALUES ('SelectQueryExtender','class','includes/database/select.inc','',0);
INSERT INTO `registry` VALUES ('SelectQueryInterface','interface','includes/database/select.inc','',0);
INSERT INTO `registry` VALUES ('ShutdownFunctionsTest','class','modules/system/system.test','system',0);
INSERT INTO `registry` VALUES ('SiteMaintenanceTestCase','class','modules/system/system.test','system',0);
INSERT INTO `registry` VALUES ('SkipDotsRecursiveDirectoryIterator','class','includes/filetransfer/filetransfer.inc','',0);
INSERT INTO `registry` VALUES ('StreamWrapperInterface','interface','includes/stream_wrappers.inc','',0);
INSERT INTO `registry` VALUES ('SummaryLengthTestCase','class','modules/node/node.test','node',0);
INSERT INTO `registry` VALUES ('SystemAdminTestCase','class','modules/system/system.test','system',0);
INSERT INTO `registry` VALUES ('SystemAuthorizeCase','class','modules/system/system.test','system',0);
INSERT INTO `registry` VALUES ('SystemBlockTestCase','class','modules/system/system.test','system',0);
INSERT INTO `registry` VALUES ('SystemIndexPhpTest','class','modules/system/system.test','system',0);
INSERT INTO `registry` VALUES ('SystemInfoAlterTestCase','class','modules/system/system.test','system',0);
INSERT INTO `registry` VALUES ('SystemMainContentFallback','class','modules/system/system.test','system',0);
INSERT INTO `registry` VALUES ('SystemQueue','class','modules/system/system.queue.inc','system',0);
INSERT INTO `registry` VALUES ('SystemThemeFunctionalTest','class','modules/system/system.test','system',0);
INSERT INTO `registry` VALUES ('SystemValidTokenTest','class','modules/system/system.test','system',0);
INSERT INTO `registry` VALUES ('TableSort','class','includes/tablesort.inc','',0);
INSERT INTO `registry` VALUES ('TestingMailSystem','class','modules/system/system.mail.inc','system',0);
INSERT INTO `registry` VALUES ('TextFieldTestCase','class','modules/field/modules/text/text.test','text',0);
INSERT INTO `registry` VALUES ('TextSummaryTestCase','class','modules/field/modules/text/text.test','text',0);
INSERT INTO `registry` VALUES ('TextTranslationTestCase','class','modules/field/modules/text/text.test','text',0);
INSERT INTO `registry` VALUES ('ThemeRegistry','class','includes/theme.inc','',0);
INSERT INTO `registry` VALUES ('ThemeUpdater','class','modules/system/system.updater.inc','system',0);
INSERT INTO `registry` VALUES ('TokenReplaceTestCase','class','modules/system/system.test','system',0);
INSERT INTO `registry` VALUES ('TokenScanTest','class','modules/system/system.test','system',0);
INSERT INTO `registry` VALUES ('TruncateQuery','class','includes/database/query.inc','',0);
INSERT INTO `registry` VALUES ('TruncateQuery_mysql','class','includes/database/mysql/query.inc','',0);
INSERT INTO `registry` VALUES ('TruncateQuery_sqlite','class','includes/database/sqlite/query.inc','',0);
INSERT INTO `registry` VALUES ('UpdateCoreTestCase','class','modules/update/update.test','update',0);
INSERT INTO `registry` VALUES ('UpdateCoreUnitTestCase','class','modules/update/update.test','update',0);
INSERT INTO `registry` VALUES ('UpdateQuery','class','includes/database/query.inc','',0);
INSERT INTO `registry` VALUES ('UpdateQuery_pgsql','class','includes/database/pgsql/query.inc','',0);
INSERT INTO `registry` VALUES ('UpdateQuery_sqlite','class','includes/database/sqlite/query.inc','',0);
INSERT INTO `registry` VALUES ('Updater','class','includes/updater.inc','',0);
INSERT INTO `registry` VALUES ('UpdaterException','class','includes/updater.inc','',0);
INSERT INTO `registry` VALUES ('UpdaterFileTransferException','class','includes/updater.inc','',0);
INSERT INTO `registry` VALUES ('UpdateScriptFunctionalTest','class','modules/system/system.test','system',0);
INSERT INTO `registry` VALUES ('UpdateTestContribCase','class','modules/update/update.test','update',0);
INSERT INTO `registry` VALUES ('UpdateTestHelper','class','modules/update/update.test','update',0);
INSERT INTO `registry` VALUES ('UpdateTestUploadCase','class','modules/update/update.test','update',0);
/*!40000 ALTER TABLE `registry` ENABLE KEYS */;
UNLOCK TABLES;

LOCK TABLES `registry_file` WRITE;
/*!40000 ALTER TABLE `registry_file` DISABLE KEYS */;
INSERT INTO `registry_file` VALUES ('includes/actions.inc','f36b066681463c7dfe189e0430cb1a89bf66f7e228cbb53cdfcd93987193f759');
INSERT INTO `registry_file` VALUES ('includes/ajax.inc','f5d608554c6b42b976d6a97e1efffe53c657e9fbb77eabb858935bfdf4276491');
INSERT INTO `registry_file` VALUES ('includes/archiver.inc','bdbb21b712a62f6b913590b609fd17cd9f3c3b77c0d21f68e71a78427ed2e3e9');
INSERT INTO `registry_file` VALUES ('includes/authorize.inc','6d64d8c21aa01eb12fc29918732e4df6b871ed06e5d41373cb95c197ed661d13');
INSERT INTO `registry_file` VALUES ('includes/batch.inc','059da9e36e1f3717f27840aae73f10dea7d6c8daf16f6520401cc1ca3b4c0388');
INSERT INTO `registry_file` VALUES ('includes/batch.queue.inc','554b2e92e1dad0f7fd5a19cb8dff7e109f10fbe2441a5692d076338ec908de0f');
INSERT INTO `registry_file` VALUES ('includes/bootstrap.inc','80e808902f62fc0417227c40eac66b81fd6f692e85ab2587bde9ef5ecc0f3ba9');
INSERT INTO `registry_file` VALUES ('includes/cache-install.inc','e7ed123c5805703c84ad2cce9c1ca46b3ce8caeeea0d8ef39a3024a4ab95fa0e');
INSERT INTO `registry_file` VALUES ('includes/cache.inc','d01e10e4c18010b6908026f3d71b72717e3272cfb91a528490eba7f339f8dd1b');
INSERT INTO `registry_file` VALUES ('includes/common.inc','a535d0b08fca17e3dac1416f31d7a5519433d1aaa02b55eebc06ecceafc84a61');
INSERT INTO `registry_file` VALUES ('includes/database/database.inc','24afaff6e1026bfe315205212cba72951240a16154250e405c4c64724e6e07cc');
INSERT INTO `registry_file` VALUES ('includes/database/log.inc','9feb5a17ae2fabcf26a96d2a634ba73da501f7bcfc3599a693d916a6971d00d1');
INSERT INTO `registry_file` VALUES ('includes/database/mysql/database.inc','d62a2d8ca103cb3b085e7f8b894a7db14c02f20d0b1ed0bd32f6534a45b4527f');
INSERT INTO `registry_file` VALUES ('includes/database/mysql/install.inc','6ae316941f771732fbbabed7e1d6b4cbb41b1f429dd097d04b3345aa15e461a0');
INSERT INTO `registry_file` VALUES ('includes/database/mysql/query.inc','0212a871646c223bf77aa26b945c77a8974855373967b5fb9fdc09f8a1de88a6');
INSERT INTO `registry_file` VALUES ('includes/database/mysql/schema.inc','6f43ac87508f868fe38ee09994fc18d69915bada0237f8ac3b717cafe8f22c6b');
INSERT INTO `registry_file` VALUES ('includes/database/prefetch.inc','b5b207a66a69ecb52ee4f4459af16a7b5eabedc87254245f37cc33bebb61c0fb');
INSERT INTO `registry_file` VALUES ('includes/database/query.inc','9171653e9710c6c0d20cff865fdead5a580367137ad4cdf81059ecc2eea61c74');
INSERT INTO `registry_file` VALUES ('includes/database/schema.inc','a98b69d33975e75f7d99cb85b20c36b7fc10e35a588e07b20c1b37500f5876ca');
INSERT INTO `registry_file` VALUES ('includes/database/select.inc','5e9cdc383564ba86cb9dcad0046990ce15415a3000e4f617d6e0f30a205b852c');
INSERT INTO `registry_file` VALUES ('includes/date.inc','18c047be64f201e16d189f1cc47ed9dcf0a145151b1ee187e90511b24e5d2b36');
INSERT INTO `registry_file` VALUES ('includes/entity.inc','3080fe3c30991a48f1f314a60d02e841d263a8f222337e5bde3be61afe41ee7a');
INSERT INTO `registry_file` VALUES ('includes/errors.inc','c8bda5b8fb4062823237d9b9ced5fb518d0a61b8ae7cee7e19ef0eba837e3d69');
INSERT INTO `registry_file` VALUES ('includes/file.inc','cf1de474b1c36b8df3254730754cd8e747c2e9daaa3dc4df6eddd7bc2b870b43');
INSERT INTO `registry_file` VALUES ('includes/file.mimetypes.inc','33266e837f4ce076378e7e8cef6c5af46446226ca4259f83e13f605856a7f147');
INSERT INTO `registry_file` VALUES ('includes/filetransfer/filetransfer.inc','fdea8ae48345ec91885ac48a9bc53daf87616271472bb7c29b7e3ce219b22034');
INSERT INTO `registry_file` VALUES ('includes/filetransfer/ftp.inc','51eb119b8e1221d598ffa6cc46c8a322aa77b49a3d8879f7fb38b7221cf7e06d');
INSERT INTO `registry_file` VALUES ('includes/filetransfer/local.inc','7cbfdb46abbdf539640db27e66fb30e5265128f31002bd0dfc3af16ae01a9492');
INSERT INTO `registry_file` VALUES ('includes/filetransfer/ssh.inc','92f1232158cb32ab04cbc93ae38ad3af04796e18f66910a9bc5ca8e437f06891');
INSERT INTO `registry_file` VALUES ('includes/form.inc','0dd082d8ca8fa99e93c33b79072efe1823a45ee8d7141ce93043c5d111136a41');
INSERT INTO `registry_file` VALUES ('includes/graph.inc','8e0e313a8bb33488f371df11fc1b58d7cf80099b886cd1003871e2c896d1b536');
INSERT INTO `registry_file` VALUES ('includes/image.inc','bcdc7e1599c02227502b9d0fe36eeb2b529b130a392bc709eb737647bd361826');
INSERT INTO `registry_file` VALUES ('includes/install.core.inc','a0585c85002e6f3d702dc505584f48b55bc13e24bee749bfe5b718fbce4847e1');
INSERT INTO `registry_file` VALUES ('includes/install.inc','480c3cfd065d3ec00f4465e1b0a0d55d6a8927e78fd6774001c30163a5c648e3');
INSERT INTO `registry_file` VALUES ('includes/iso.inc','0ce4c225edcfa9f037703bc7dd09d4e268a69bcc90e55da0a3f04c502bd2f349');
INSERT INTO `registry_file` VALUES ('includes/json-encode.inc','02a822a652d00151f79db9aa9e171c310b69b93a12f549bc2ce00533a8efa14e');
INSERT INTO `registry_file` VALUES ('includes/language.inc','4dd521af07e0ca7bf97ff145f4bd3a218acf0d8b94964e72f11212bb8af8d66e');
INSERT INTO `registry_file` VALUES ('includes/locale.inc','b250f375b93ffe3749f946e0ad475065c914af23e388d68e5c5df161590f086a');
INSERT INTO `registry_file` VALUES ('includes/lock.inc','a181c8bd4f88d292a0a73b9f1fbd727e3314f66ec3631f288e6b9a54ba2b70fa');
INSERT INTO `registry_file` VALUES ('includes/mail.inc','d9fb2b99025745cbb73ebcfc7ac12df100508b9273ce35c433deacf12dd6a13a');
INSERT INTO `registry_file` VALUES ('includes/menu.inc','c9ff3c7db04b7e01d0d19b5e47d9fb209799f2ae6584167235b957d22542e526');
INSERT INTO `registry_file` VALUES ('includes/module.inc','f63ab8cec01f932d7abfc2d09d91ba322e333f4ff447088ab0db4d16b5d9f676');
INSERT INTO `registry_file` VALUES ('includes/pager.inc','6f9494b85c07a2cc3be4e54aff2d2757485238c476a7da084d25bde1d88be6d8');
INSERT INTO `registry_file` VALUES ('includes/password.inc','fd9a1c94fe5a0fa7c7049a2435c7280b1d666b2074595010e3c492dd15712775');
INSERT INTO `registry_file` VALUES ('includes/path.inc','74bf05f3c68b0218730abf3e539fcf08b271959c8f4611940d05124f34a6a66f');
INSERT INTO `registry_file` VALUES ('includes/registry.inc','c225de772f86eebd21b0b52fa8fcc6671e05fa2374cedb3164f7397f27d3c88d');
INSERT INTO `registry_file` VALUES ('includes/session.inc','7548621ae4c273179a76eba41aa58b740100613bc015ad388a5c30132b61e34b');
INSERT INTO `registry_file` VALUES ('includes/stream_wrappers.inc','4f1feb774a8dbc04ca382fa052f59e58039c7261625f3df29987d6b31f08d92d');
INSERT INTO `registry_file` VALUES ('includes/tablesort.inc','2d88768a544829595dd6cda2a5eb008bedb730f36bba6dfe005d9ddd999d5c0f');
INSERT INTO `registry_file` VALUES ('includes/theme.inc','ab2a805bb52a54dc762f314bbba6b55b959734a87e8f96119435d08b08e6fe1f');
INSERT INTO `registry_file` VALUES ('includes/theme.maintenance.inc','39f068b3eee4d10a90d6aa3c86db587b6d25844c2919d418d34d133cfe330f5a');
INSERT INTO `registry_file` VALUES ('includes/token.inc','5e7898cd78689e2c291ed3cd8f41c032075656896f1db57e49217aac19ae0428');
INSERT INTO `registry_file` VALUES ('includes/unicode.entities.inc','2b858138596d961fbaa4c6e3986e409921df7f76b6ee1b109c4af5970f1e0f54');
INSERT INTO `registry_file` VALUES ('includes/unicode.inc','e18772dafe0f80eb139fcfc582fef1704ba9f730647057d4f4841d6a6e4066ca');
INSERT INTO `registry_file` VALUES ('includes/update.inc','177ce24362efc7f28b384c90a09c3e485396bbd18c3721d4b21e57dd1733bd92');
INSERT INTO `registry_file` VALUES ('includes/updater.inc','d2da0e74ed86e93c209f16069f3d32e1a134ceb6c06a0044f78e841a1b54e380');
INSERT INTO `registry_file` VALUES ('includes/utility.inc','3458fd2b55ab004dd0cc529b8e58af12916e8bd36653b072bdd820b26b907ed5');
INSERT INTO `registry_file` VALUES ('includes/xmlrpc.inc','ea24176ec445c440ba0c825fc7b04a31b440288df8ef02081560dc418e34e659');
INSERT INTO `registry_file` VALUES ('includes/xmlrpcs.inc','741aa8d6fcc6c45a9409064f52351f7999b7c702d73def8da44de2567946598a');
INSERT INTO `registry_file` VALUES ('modules/block/block.test','df1b364688b46345523dfcb95c0c48352d6a4edbc66597890d29b9b0d7866e86');
INSERT INTO `registry_file` VALUES ('modules/field/field.attach.inc','2df4687b5ec078c4893dc1fea514f67524fd5293de717b9e05caf977e5ae2327');
INSERT INTO `registry_file` VALUES ('modules/field/field.info.class.inc','a6f2f418552dba0e03f57ee812a6f0f63bbfe4bf81fe805d51ecec47ef84b845');
INSERT INTO `registry_file` VALUES ('modules/field/field.module','2ec1a3ec060504467c3065426a5a1eca8e2c894cb4d4480616bca60fe4b2faf2');
INSERT INTO `registry_file` VALUES ('modules/system/system.archiver.inc','faa849f3e646a910ab82fd6c8bbf0a4e6b8c60725d7ba81ec0556bd716616cd1');
INSERT INTO `registry_file` VALUES ('modules/system/system.mail.inc','d31e1769f5defbe5f27dc68f641ab80fb8d3de92f6e895f4c654ec05fc7e5f0f');
INSERT INTO `registry_file` VALUES ('modules/system/system.queue.inc','ef00fd41ca86de386fa134d5bc1d816f9af550cf0e1334a5c0ade3119688ca3c');
INSERT INTO `registry_file` VALUES ('modules/system/system.tar.inc','8a31d91f7b3cd7eac25b3fa46e1ed9a8527c39718ba76c3f8c0bbbeaa3aa4086');
INSERT INTO `registry_file` VALUES ('modules/system/system.test','ad3c68f2cacfe6a99c065edc9aca05a22bdbc74ff6158e9918255b4633134ab4');
INSERT INTO `registry_file` VALUES ('modules/system/system.updater.inc','338cf14cb691ba16ee551b3b9e0fa4f579a2f25c964130658236726d17563b6a');
/*!40000 ALTER TABLE `registry_file` ENABLE KEYS */;
UNLOCK TABLES;

LOCK TABLES `system` WRITE;
/*!40000 ALTER TABLE `system` DISABLE KEYS */;
INSERT INTO `system` VALUES ('modules/aggregator/aggregator.module','aggregator','module','',0,0,-1,0,0x613A31343A7B733A343A226E616D65223B733A31303A2241676772656761746F72223B733A31313A226465736372697074696F6E223B733A35373A22416767726567617465732073796E6469636174656420636F6E74656E7420285253532C205244462C20616E642041746F6D206665656473292E223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A353A2266696C6573223B613A313A7B693A303B733A31353A2261676772656761746F722E74657374223B7D733A393A22636F6E666967757265223B733A34313A2261646D696E2F636F6E6669672F73657276696365732F61676772656761746F722F73657474696E6773223B733A31313A227374796C65736865657473223B613A313A7B733A333A22616C6C223B613A313A7B733A31343A2261676772656761746F722E637373223B733A33333A226D6F64756C65732F61676772656761746F722F61676772656761746F722E637373223B7D7D733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A31323A22646570656E64656E63696573223B613A303A7B7D733A333A22706870223B733A353A22352E322E34223B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/block/block.module','block','module','',1,0,7009,-5,0x613A31333A7B733A343A226E616D65223B733A353A22426C6F636B223B733A31313A226465736372697074696F6E223B733A3134303A22436F6E74726F6C73207468652076697375616C206275696C64696E6720626C6F636B732061207061676520697320636F6E737472756374656420776974682E20426C6F636B732061726520626F786573206F6620636F6E74656E742072656E646572656420696E746F20616E20617265612C206F7220726567696F6E2C206F6620612077656220706167652E223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A353A2266696C6573223B613A313A7B693A303B733A31303A22626C6F636B2E74657374223B7D733A393A22636F6E666967757265223B733A32313A2261646D696E2F7374727563747572652F626C6F636B223B733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A31323A22646570656E64656E63696573223B613A303A7B7D733A333A22706870223B733A353A22352E322E34223B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/blog/blog.module','blog','module','',0,0,-1,0,0x613A31323A7B733A343A226E616D65223B733A343A22426C6F67223B733A31313A226465736372697074696F6E223B733A32353A22456E61626C6573206D756C74692D7573657220626C6F67732E223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A353A2266696C6573223B613A313A7B693A303B733A393A22626C6F672E74657374223B7D733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A31323A22646570656E64656E63696573223B613A303A7B7D733A333A22706870223B733A353A22352E322E34223B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/book/book.module','book','module','',0,0,-1,0,0x613A31343A7B733A343A226E616D65223B733A343A22426F6F6B223B733A31313A226465736372697074696F6E223B733A36363A22416C6C6F777320757365727320746F2063726561746520616E64206F7267616E697A652072656C6174656420636F6E74656E7420696E20616E206F75746C696E652E223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A353A2266696C6573223B613A313A7B693A303B733A393A22626F6F6B2E74657374223B7D733A393A22636F6E666967757265223B733A32373A2261646D696E2F636F6E74656E742F626F6F6B2F73657474696E6773223B733A31313A227374796C65736865657473223B613A313A7B733A333A22616C6C223B613A313A7B733A383A22626F6F6B2E637373223B733A32313A226D6F64756C65732F626F6F6B2F626F6F6B2E637373223B7D7D733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A31323A22646570656E64656E63696573223B613A303A7B7D733A333A22706870223B733A353A22352E322E34223B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/color/color.module','color','module','',0,0,-1,0,0x613A31323A7B733A343A226E616D65223B733A353A22436F6C6F72223B733A31313A226465736372697074696F6E223B733A37303A22416C6C6F77732061646D696E6973747261746F727320746F206368616E67652074686520636F6C6F7220736368656D65206F6620636F6D70617469626C65207468656D65732E223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A353A2266696C6573223B613A313A7B693A303B733A31303A22636F6C6F722E74657374223B7D733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A31323A22646570656E64656E63696573223B613A303A7B7D733A333A22706870223B733A353A22352E322E34223B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/comment/comment.module','comment','module','',0,0,-1,0,0x613A31343A7B733A343A226E616D65223B733A373A22436F6D6D656E74223B733A31313A226465736372697074696F6E223B733A35373A22416C6C6F777320757365727320746F20636F6D6D656E74206F6E20616E642064697363757373207075626C697368656420636F6E74656E742E223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A31323A22646570656E64656E63696573223B613A313A7B693A303B733A343A2274657874223B7D733A353A2266696C6573223B613A323A7B693A303B733A31343A22636F6D6D656E742E6D6F64756C65223B693A313B733A31323A22636F6D6D656E742E74657374223B7D733A393A22636F6E666967757265223B733A32313A2261646D696E2F636F6E74656E742F636F6D6D656E74223B733A31313A227374796C65736865657473223B613A313A7B733A333A22616C6C223B613A313A7B733A31313A22636F6D6D656E742E637373223B733A32373A226D6F64756C65732F636F6D6D656E742F636F6D6D656E742E637373223B7D7D733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A333A22706870223B733A353A22352E322E34223B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/contact/contact.module','contact','module','',0,0,-1,0,0x613A31333A7B733A343A226E616D65223B733A373A22436F6E74616374223B733A31313A226465736372697074696F6E223B733A36313A22456E61626C65732074686520757365206F6620626F746820706572736F6E616C20616E6420736974652D7769646520636F6E7461637420666F726D732E223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A353A2266696C6573223B613A313A7B693A303B733A31323A22636F6E746163742E74657374223B7D733A393A22636F6E666967757265223B733A32333A2261646D696E2F7374727563747572652F636F6E74616374223B733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A31323A22646570656E64656E63696573223B613A303A7B7D733A333A22706870223B733A353A22352E322E34223B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/contextual/contextual.module','contextual','module','',0,0,-1,0,0x613A31323A7B733A343A226E616D65223B733A31363A22436F6E7465787475616C206C696E6B73223B733A31313A226465736372697074696F6E223B733A37353A2250726F766964657320636F6E7465787475616C206C696E6B7320746F20706572666F726D20616374696F6E732072656C6174656420746F20656C656D656E7473206F6E206120706167652E223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A353A2266696C6573223B613A313A7B693A303B733A31353A22636F6E7465787475616C2E74657374223B7D733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A31323A22646570656E64656E63696573223B613A303A7B7D733A333A22706870223B733A353A22352E322E34223B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/dashboard/dashboard.module','dashboard','module','',0,0,-1,0,0x613A31333A7B733A343A226E616D65223B733A393A2244617368626F617264223B733A31313A226465736372697074696F6E223B733A3133363A2250726F766964657320612064617368626F617264207061676520696E207468652061646D696E69737472617469766520696E7465726661636520666F72206F7267616E697A696E672061646D696E697374726174697665207461736B7320616E6420747261636B696E6720696E666F726D6174696F6E2077697468696E20796F757220736974652E223B733A343A22636F7265223B733A333A22372E78223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A353A2266696C6573223B613A313A7B693A303B733A31343A2264617368626F6172642E74657374223B7D733A31323A22646570656E64656E63696573223B613A313A7B693A303B733A353A22626C6F636B223B7D733A393A22636F6E666967757265223B733A32353A2261646D696E2F64617368626F6172642F637573746F6D697A65223B733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A333A22706870223B733A353A22352E322E34223B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/dblog/dblog.module','dblog','module','',0,0,-1,0,0x613A31323A7B733A343A226E616D65223B733A31363A224461746162617365206C6F6767696E67223B733A31313A226465736372697074696F6E223B733A34373A224C6F677320616E64207265636F7264732073797374656D206576656E747320746F207468652064617461626173652E223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A353A2266696C6573223B613A313A7B693A303B733A31303A2264626C6F672E74657374223B7D733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A31323A22646570656E64656E63696573223B613A303A7B7D733A333A22706870223B733A353A22352E322E34223B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/field_ui/field_ui.module','field_ui','module','',0,0,-1,0,0x613A31323A7B733A343A226E616D65223B733A383A224669656C64205549223B733A31313A226465736372697074696F6E223B733A33333A225573657220696E7465726661636520666F7220746865204669656C64204150492E223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A31323A22646570656E64656E63696573223B613A313A7B693A303B733A353A226669656C64223B7D733A353A2266696C6573223B613A313A7B693A303B733A31333A226669656C645F75692E74657374223B7D733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A333A22706870223B733A353A22352E322E34223B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/field/field.module','field','module','',1,0,7003,0,0x613A31343A7B733A343A226E616D65223B733A353A224669656C64223B733A31313A226465736372697074696F6E223B733A35373A224669656C642041504920746F20616464206669656C647320746F20656E746974696573206C696B65206E6F64657320616E642075736572732E223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A353A2266696C6573223B613A343A7B693A303B733A31323A226669656C642E6D6F64756C65223B693A313B733A31363A226669656C642E6174746163682E696E63223B693A323B733A32303A226669656C642E696E666F2E636C6173732E696E63223B693A333B733A31363A2274657374732F6669656C642E74657374223B7D733A31323A22646570656E64656E63696573223B613A313A7B693A303B733A31373A226669656C645F73716C5F73746F72616765223B7D733A383A227265717569726564223B623A313B733A31313A227374796C65736865657473223B613A313A7B733A333A22616C6C223B613A313A7B733A31353A227468656D652F6669656C642E637373223B733A32393A226D6F64756C65732F6669656C642F7468656D652F6669656C642E637373223B7D7D733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A333A22706870223B733A353A22352E322E34223B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/field/modules/field_sql_storage/field_sql_storage.module','field_sql_storage','module','',1,0,7002,0,0x613A31333A7B733A343A226E616D65223B733A31373A224669656C642053514C2073746F72616765223B733A31313A226465736372697074696F6E223B733A33373A2253746F726573206669656C64206461746120696E20616E2053514C2064617461626173652E223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A31323A22646570656E64656E63696573223B613A313A7B693A303B733A353A226669656C64223B7D733A353A2266696C6573223B613A313A7B693A303B733A32323A226669656C645F73716C5F73746F726167652E74657374223B7D733A383A227265717569726564223B623A313B733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A333A22706870223B733A353A22352E322E34223B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/field/modules/list/list.module','list','module','',0,0,-1,0,0x613A31323A7B733A343A226E616D65223B733A343A224C697374223B733A31313A226465736372697074696F6E223B733A36393A22446566696E6573206C697374206669656C642074797065732E205573652077697468204F7074696F6E7320746F206372656174652073656C656374696F6E206C697374732E223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A31323A22646570656E64656E63696573223B613A323A7B693A303B733A353A226669656C64223B693A313B733A373A226F7074696F6E73223B7D733A353A2266696C6573223B613A313A7B693A303B733A31353A2274657374732F6C6973742E74657374223B7D733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A333A22706870223B733A353A22352E322E34223B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/field/modules/number/number.module','number','module','',0,0,-1,0,0x613A31323A7B733A343A226E616D65223B733A363A224E756D626572223B733A31313A226465736372697074696F6E223B733A32383A22446566696E6573206E756D65726963206669656C642074797065732E223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A31323A22646570656E64656E63696573223B613A313A7B693A303B733A353A226669656C64223B7D733A353A2266696C6573223B613A313A7B693A303B733A31313A226E756D6265722E74657374223B7D733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A333A22706870223B733A353A22352E322E34223B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/field/modules/options/options.module','options','module','',0,0,-1,0,0x613A31323A7B733A343A226E616D65223B733A373A224F7074696F6E73223B733A31313A226465736372697074696F6E223B733A38323A22446566696E65732073656C656374696F6E2C20636865636B20626F7820616E6420726164696F20627574746F6E207769646765747320666F72207465787420616E64206E756D65726963206669656C64732E223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A31323A22646570656E64656E63696573223B613A313A7B693A303B733A353A226669656C64223B7D733A353A2266696C6573223B613A313A7B693A303B733A31323A226F7074696F6E732E74657374223B7D733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A333A22706870223B733A353A22352E322E34223B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/field/modules/text/text.module','text','module','',1,0,7000,0,0x613A31333A7B733A343A226E616D65223B733A343A2254657874223B733A31313A226465736372697074696F6E223B733A33323A22446566696E65732073696D706C652074657874206669656C642074797065732E223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A31323A22646570656E64656E63696573223B613A313A7B693A303B733A353A226669656C64223B7D733A353A2266696C6573223B613A313A7B693A303B733A393A22746578742E74657374223B7D733A383A227265717569726564223B623A313B733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A333A22706870223B733A353A22352E322E34223B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/file/file.module','file','module','',1,0,0,0,0x613A31323A7B733A343A226E616D65223B733A343A2246696C65223B733A31313A226465736372697074696F6E223B733A32363A22446566696E657320612066696C65206669656C6420747970652E223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A31323A22646570656E64656E63696573223B613A313A7B693A303B733A353A226669656C64223B7D733A353A2266696C6573223B613A313A7B693A303B733A31353A2274657374732F66696C652E74657374223B7D733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A333A22706870223B733A353A22352E322E34223B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/filter/filter.module','filter','module','',1,0,7010,0,0x613A31343A7B733A343A226E616D65223B733A363A2246696C746572223B733A31313A226465736372697074696F6E223B733A34333A2246696C7465727320636F6E74656E7420696E207072657061726174696F6E20666F7220646973706C61792E223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A353A2266696C6573223B613A313A7B693A303B733A31313A2266696C7465722E74657374223B7D733A383A227265717569726564223B623A313B733A393A22636F6E666967757265223B733A32383A2261646D696E2F636F6E6669672F636F6E74656E742F666F726D617473223B733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A31323A22646570656E64656E63696573223B613A303A7B7D733A333A22706870223B733A353A22352E322E34223B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/forum/forum.module','forum','module','',0,0,-1,0,0x613A31343A7B733A343A226E616D65223B733A353A22466F72756D223B733A31313A226465736372697074696F6E223B733A32373A2250726F76696465732064697363757373696F6E20666F72756D732E223B733A31323A22646570656E64656E63696573223B613A323A7B693A303B733A383A227461786F6E6F6D79223B693A313B733A373A22636F6D6D656E74223B7D733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A353A2266696C6573223B613A313A7B693A303B733A31303A22666F72756D2E74657374223B7D733A393A22636F6E666967757265223B733A32313A2261646D696E2F7374727563747572652F666F72756D223B733A31313A227374796C65736865657473223B613A313A7B733A333A22616C6C223B613A313A7B733A393A22666F72756D2E637373223B733A32333A226D6F64756C65732F666F72756D2F666F72756D2E637373223B7D7D733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A333A22706870223B733A353A22352E322E34223B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/help/help.module','help','module','',0,0,-1,0,0x613A31323A7B733A343A226E616D65223B733A343A2248656C70223B733A31313A226465736372697074696F6E223B733A33353A224D616E616765732074686520646973706C6179206F66206F6E6C696E652068656C702E223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A353A2266696C6573223B613A313A7B693A303B733A393A2268656C702E74657374223B7D733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A31323A22646570656E64656E63696573223B613A303A7B7D733A333A22706870223B733A353A22352E322E34223B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/image/image.module','image','module','',0,0,-1,0,0x613A31333A7B733A343A226E616D65223B733A353A22496D616765223B733A31313A226465736372697074696F6E223B733A33343A2250726F766964657320696D616765206D616E6970756C6174696F6E20746F6F6C732E223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A31323A22646570656E64656E63696573223B613A313A7B693A303B733A343A2266696C65223B7D733A353A2266696C6573223B613A313A7B693A303B733A31303A22696D6167652E74657374223B7D733A393A22636F6E666967757265223B733A33313A2261646D696E2F636F6E6669672F6D656469612F696D6167652D7374796C6573223B733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A333A22706870223B733A353A22352E322E34223B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/locale/locale.module','locale','module','',0,0,-1,0,0x613A31333A7B733A343A226E616D65223B733A363A224C6F63616C65223B733A31313A226465736372697074696F6E223B733A3131393A2241646473206C616E67756167652068616E646C696E672066756E6374696F6E616C69747920616E6420656E61626C657320746865207472616E736C6174696F6E206F6620746865207573657220696E7465726661636520746F206C616E677561676573206F74686572207468616E20456E676C6973682E223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A353A2266696C6573223B613A313A7B693A303B733A31313A226C6F63616C652E74657374223B7D733A393A22636F6E666967757265223B733A33303A2261646D696E2F636F6E6669672F726567696F6E616C2F6C616E6775616765223B733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A31323A22646570656E64656E63696573223B613A303A7B7D733A333A22706870223B733A353A22352E322E34223B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/menu/menu.module','menu','module','',0,0,-1,0,0x613A31333A7B733A343A226E616D65223B733A343A224D656E75223B733A31313A226465736372697074696F6E223B733A36303A22416C6C6F77732061646D696E6973747261746F727320746F20637573746F6D697A65207468652073697465206E617669676174696F6E206D656E752E223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A353A2266696C6573223B613A313A7B693A303B733A393A226D656E752E74657374223B7D733A393A22636F6E666967757265223B733A32303A2261646D696E2F7374727563747572652F6D656E75223B733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A31323A22646570656E64656E63696573223B613A303A7B7D733A333A22706870223B733A353A22352E322E34223B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/node/node.module','node','module','',1,0,7014,0,0x613A31353A7B733A343A226E616D65223B733A343A224E6F6465223B733A31313A226465736372697074696F6E223B733A36363A22416C6C6F777320636F6E74656E7420746F206265207375626D697474656420746F20746865207369746520616E6420646973706C61796564206F6E2070616765732E223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A353A2266696C6573223B613A323A7B693A303B733A31313A226E6F64652E6D6F64756C65223B693A313B733A393A226E6F64652E74657374223B7D733A383A227265717569726564223B623A313B733A393A22636F6E666967757265223B733A32313A2261646D696E2F7374727563747572652F7479706573223B733A31313A227374796C65736865657473223B613A313A7B733A333A22616C6C223B613A313A7B733A383A226E6F64652E637373223B733A32313A226D6F64756C65732F6E6F64652F6E6F64652E637373223B7D7D733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A31323A22646570656E64656E63696573223B613A303A7B7D733A333A22706870223B733A353A22352E322E34223B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/openid/openid.module','openid','module','',0,0,-1,0,0x613A31323A7B733A343A226E616D65223B733A363A224F70656E4944223B733A31313A226465736372697074696F6E223B733A34383A22416C6C6F777320757365727320746F206C6F6720696E746F20796F75722073697465207573696E67204F70656E49442E223B733A373A2276657273696F6E223B733A343A22372E3334223B733A373A227061636B616765223B733A343A22436F7265223B733A343A22636F7265223B733A333A22372E78223B733A353A2266696C6573223B613A313A7B693A303B733A31313A226F70656E69642E74657374223B7D733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A31323A22646570656E64656E63696573223B613A303A7B7D733A333A22706870223B733A353A22352E322E34223B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/overlay/overlay.module','overlay','module','',0,0,-1,0,0x613A31323A7B733A343A226E616D65223B733A373A224F7665726C6179223B733A31313A226465736372697074696F6E223B733A35393A22446973706C617973207468652044727570616C2061646D696E697374726174696F6E20696E7465726661636520696E20616E206F7665726C61792E223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A31323A22646570656E64656E63696573223B613A303A7B7D733A333A22706870223B733A353A22352E322E34223B733A353A2266696C6573223B613A303A7B7D733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/path/path.module','path','module','',0,0,-1,0,0x613A31333A7B733A343A226E616D65223B733A343A2250617468223B733A31313A226465736372697074696F6E223B733A32383A22416C6C6F777320757365727320746F2072656E616D652055524C732E223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A353A2266696C6573223B613A313A7B693A303B733A393A22706174682E74657374223B7D733A393A22636F6E666967757265223B733A32343A2261646D696E2F636F6E6669672F7365617263682F70617468223B733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A31323A22646570656E64656E63696573223B613A303A7B7D733A333A22706870223B733A353A22352E322E34223B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/php/php.module','php','module','',0,0,-1,0,0x613A31323A7B733A343A226E616D65223B733A31303A225048502066696C746572223B733A31313A226465736372697074696F6E223B733A35303A22416C6C6F777320656D6265646465642050485020636F64652F736E69707065747320746F206265206576616C75617465642E223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A353A2266696C6573223B613A313A7B693A303B733A383A227068702E74657374223B7D733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A31323A22646570656E64656E63696573223B613A303A7B7D733A333A22706870223B733A353A22352E322E34223B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/poll/poll.module','poll','module','',0,0,-1,0,0x613A31333A7B733A343A226E616D65223B733A343A22506F6C6C223B733A31313A226465736372697074696F6E223B733A39353A22416C6C6F777320796F7572207369746520746F206361707475726520766F746573206F6E20646966666572656E7420746F7069637320696E2074686520666F726D206F66206D756C7469706C652063686F696365207175657374696F6E732E223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A353A2266696C6573223B613A313A7B693A303B733A393A22706F6C6C2E74657374223B7D733A31313A227374796C65736865657473223B613A313A7B733A333A22616C6C223B613A313A7B733A383A22706F6C6C2E637373223B733A32313A226D6F64756C65732F706F6C6C2F706F6C6C2E637373223B7D7D733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A31323A22646570656E64656E63696573223B613A303A7B7D733A333A22706870223B733A353A22352E322E34223B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/profile/profile.module','profile','module','',0,0,-1,0,0x613A31343A7B733A343A226E616D65223B733A373A2250726F66696C65223B733A31313A226465736372697074696F6E223B733A33363A22537570706F72747320636F6E666967757261626C6520757365722070726F66696C65732E223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A353A2266696C6573223B613A313A7B693A303B733A31323A2270726F66696C652E74657374223B7D733A393A22636F6E666967757265223B733A32373A2261646D696E2F636F6E6669672F70656F706C652F70726F66696C65223B733A363A2268696464656E223B623A313B733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A31323A22646570656E64656E63696573223B613A303A7B7D733A333A22706870223B733A353A22352E322E34223B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/rdf/rdf.module','rdf','module','',0,0,-1,0,0x613A31323A7B733A343A226E616D65223B733A333A22524446223B733A31313A226465736372697074696F6E223B733A3134383A22456E72696368657320796F757220636F6E74656E742077697468206D6574616461746120746F206C6574206F74686572206170706C69636174696F6E732028652E672E2073656172636820656E67696E65732C2061676772656761746F7273292062657474657220756E6465727374616E64206974732072656C6174696F6E736869707320616E6420617474726962757465732E223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A353A2266696C6573223B613A313A7B693A303B733A383A227264662E74657374223B7D733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A31323A22646570656E64656E63696573223B613A303A7B7D733A333A22706870223B733A353A22352E322E34223B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/search/search.module','search','module','',0,0,-1,0,0x613A31343A7B733A343A226E616D65223B733A363A22536561726368223B733A31313A226465736372697074696F6E223B733A33363A22456E61626C657320736974652D77696465206B6579776F726420736561726368696E672E223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A353A2266696C6573223B613A323A7B693A303B733A31393A227365617263682E657874656E6465722E696E63223B693A313B733A31313A227365617263682E74657374223B7D733A393A22636F6E666967757265223B733A32383A2261646D696E2F636F6E6669672F7365617263682F73657474696E6773223B733A31313A227374796C65736865657473223B613A313A7B733A333A22616C6C223B613A313A7B733A31303A227365617263682E637373223B733A32353A226D6F64756C65732F7365617263682F7365617263682E637373223B7D7D733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A31323A22646570656E64656E63696573223B613A303A7B7D733A333A22706870223B733A353A22352E322E34223B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/shortcut/shortcut.module','shortcut','module','',0,0,-1,0,0x613A31333A7B733A343A226E616D65223B733A383A2253686F7274637574223B733A31313A226465736372697074696F6E223B733A36303A22416C6C6F777320757365727320746F206D616E61676520637573746F6D697A61626C65206C69737473206F662073686F7274637574206C696E6B732E223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A353A2266696C6573223B613A313A7B693A303B733A31333A2273686F72746375742E74657374223B7D733A393A22636F6E666967757265223B733A33363A2261646D696E2F636F6E6669672F757365722D696E746572666163652F73686F7274637574223B733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A31323A22646570656E64656E63696573223B613A303A7B7D733A333A22706870223B733A353A22352E322E34223B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/statistics/statistics.module','statistics','module','',0,0,-1,0,0x613A31333A7B733A343A226E616D65223B733A31303A2253746174697374696373223B733A31313A226465736372697074696F6E223B733A33373A224C6F677320616363657373207374617469737469637320666F7220796F757220736974652E223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A353A2266696C6573223B613A313A7B693A303B733A31353A22737461746973746963732E74657374223B7D733A393A22636F6E666967757265223B733A33303A2261646D696E2F636F6E6669672F73797374656D2F73746174697374696373223B733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A31323A22646570656E64656E63696573223B613A303A7B7D733A333A22706870223B733A353A22352E322E34223B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/syslog/syslog.module','syslog','module','',0,0,-1,0,0x613A31333A7B733A343A226E616D65223B733A363A225379736C6F67223B733A31313A226465736372697074696F6E223B733A34313A224C6F677320616E64207265636F7264732073797374656D206576656E747320746F207379736C6F672E223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A353A2266696C6573223B613A313A7B693A303B733A31313A227379736C6F672E74657374223B7D733A393A22636F6E666967757265223B733A33323A2261646D696E2F636F6E6669672F646576656C6F706D656E742F6C6F6767696E67223B733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A31323A22646570656E64656E63696573223B613A303A7B7D733A333A22706870223B733A353A22352E322E34223B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/system/system.module','system','module','',1,0,7079,0,0x613A31343A7B733A343A226E616D65223B733A363A2253797374656D223B733A31313A226465736372697074696F6E223B733A35343A2248616E646C65732067656E6572616C207369746520636F6E66696775726174696F6E20666F722061646D696E6973747261746F72732E223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A353A2266696C6573223B613A363A7B693A303B733A31393A2273797374656D2E61726368697665722E696E63223B693A313B733A31353A2273797374656D2E6D61696C2E696E63223B693A323B733A31363A2273797374656D2E71756575652E696E63223B693A333B733A31343A2273797374656D2E7461722E696E63223B693A343B733A31383A2273797374656D2E757064617465722E696E63223B693A353B733A31313A2273797374656D2E74657374223B7D733A383A227265717569726564223B623A313B733A393A22636F6E666967757265223B733A31393A2261646D696E2F636F6E6669672F73797374656D223B733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A31323A22646570656E64656E63696573223B613A303A7B7D733A333A22706870223B733A353A22352E322E34223B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/taxonomy/taxonomy.module','taxonomy','module','',0,0,-1,0,0x613A31333A7B733A343A226E616D65223B733A383A225461786F6E6F6D79223B733A31313A226465736372697074696F6E223B733A33383A22456E61626C6573207468652063617465676F72697A6174696F6E206F6620636F6E74656E742E223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A31323A22646570656E64656E63696573223B613A313A7B693A303B733A373A226F7074696F6E73223B7D733A353A2266696C6573223B613A323A7B693A303B733A31353A227461786F6E6F6D792E6D6F64756C65223B693A313B733A31333A227461786F6E6F6D792E74657374223B7D733A393A22636F6E666967757265223B733A32343A2261646D696E2F7374727563747572652F7461786F6E6F6D79223B733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A333A22706870223B733A353A22352E322E34223B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/toolbar/toolbar.module','toolbar','module','',0,0,0,0,0x613A31323A7B733A343A226E616D65223B733A373A22546F6F6C626172223B733A31313A226465736372697074696F6E223B733A39393A2250726F7669646573206120746F6F6C62617220746861742073686F77732074686520746F702D6C6576656C2061646D696E697374726174696F6E206D656E75206974656D7320616E64206C696E6B732066726F6D206F74686572206D6F64756C65732E223B733A343A22636F7265223B733A333A22372E78223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A31323A22646570656E64656E63696573223B613A303A7B7D733A333A22706870223B733A353A22352E322E34223B733A353A2266696C6573223B613A303A7B7D733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/tracker/tracker.module','tracker','module','',0,0,-1,0,0x613A31323A7B733A343A226E616D65223B733A373A22547261636B6572223B733A31313A226465736372697074696F6E223B733A34353A22456E61626C657320747261636B696E67206F6620726563656E7420636F6E74656E7420666F722075736572732E223B733A31323A22646570656E64656E63696573223B613A313A7B693A303B733A373A22636F6D6D656E74223B7D733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A353A2266696C6573223B613A313A7B693A303B733A31323A22747261636B65722E74657374223B7D733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A333A22706870223B733A353A22352E322E34223B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/translation/translation.module','translation','module','',0,0,-1,0,0x613A31323A7B733A343A226E616D65223B733A31393A22436F6E74656E74207472616E736C6174696F6E223B733A31313A226465736372697074696F6E223B733A35373A22416C6C6F777320636F6E74656E7420746F206265207472616E736C6174656420696E746F20646966666572656E74206C616E6775616765732E223B733A31323A22646570656E64656E63696573223B613A313A7B693A303B733A363A226C6F63616C65223B7D733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A353A2266696C6573223B613A313A7B693A303B733A31363A227472616E736C6174696F6E2E74657374223B7D733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A333A22706870223B733A353A22352E322E34223B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/trigger/trigger.module','trigger','module','',0,0,-1,0,0x613A31333A7B733A343A226E616D65223B733A373A2254726967676572223B733A31313A226465736372697074696F6E223B733A39303A22456E61626C657320616374696F6E7320746F206265206669726564206F6E206365727461696E2073797374656D206576656E74732C2073756368206173207768656E206E657720636F6E74656E7420697320637265617465642E223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A353A2266696C6573223B613A313A7B693A303B733A31323A22747269676765722E74657374223B7D733A393A22636F6E666967757265223B733A32333A2261646D696E2F7374727563747572652F74726967676572223B733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A31323A22646570656E64656E63696573223B613A303A7B7D733A333A22706870223B733A353A22352E322E34223B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/update/update.module','update','module','',1,0,7001,0,0x613A31333A7B733A343A226E616D65223B733A31343A22557064617465206D616E61676572223B733A31313A226465736372697074696F6E223B733A3130343A22436865636B7320666F7220617661696C61626C6520757064617465732C20616E642063616E207365637572656C7920696E7374616C6C206F7220757064617465206D6F64756C657320616E64207468656D65732076696120612077656220696E746572666163652E223B733A373A2276657273696F6E223B733A343A22372E3334223B733A373A227061636B616765223B733A343A22436F7265223B733A343A22636F7265223B733A333A22372E78223B733A353A2266696C6573223B613A313A7B693A303B733A31313A227570646174652E74657374223B7D733A393A22636F6E666967757265223B733A33303A2261646D696E2F7265706F7274732F757064617465732F73657474696E6773223B733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A31323A22646570656E64656E63696573223B613A303A7B7D733A333A22706870223B733A353A22352E322E34223B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('modules/user/user.module','user','module','',0,0,7018,0,0x613A31353A7B733A343A226E616D65223B733A343A2255736572223B733A31313A226465736372697074696F6E223B733A34373A224D616E6167657320746865207573657220726567697374726174696F6E20616E64206C6F67696E2073797374656D2E223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A353A2266696C6573223B613A323A7B693A303B733A31313A22757365722E6D6F64756C65223B693A313B733A393A22757365722E74657374223B7D733A383A227265717569726564223B623A313B733A393A22636F6E666967757265223B733A31393A2261646D696E2F636F6E6669672F70656F706C65223B733A31313A227374796C65736865657473223B613A313A7B733A333A22616C6C223B613A313A7B733A383A22757365722E637373223B733A32313A226D6F64756C65732F757365722F757365722E637373223B7D7D733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A31323A22646570656E64656E63696573223B613A303A7B7D733A333A22706870223B733A353A22352E322E34223B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('profiles/standard/standard.profile','standard','module','',1,0,0,1000,0x613A31353A7B733A343A226E616D65223B733A383A225374616E64617264223B733A31313A226465736372697074696F6E223B733A35313A22496E7374616C6C207769746820636F6D6D6F6E6C792075736564206665617475726573207072652D636F6E666967757265642E223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A31323A22646570656E64656E63696573223B613A32313A7B693A303B733A353A22626C6F636B223B693A313B733A353A22636F6C6F72223B693A323B733A373A22636F6D6D656E74223B693A333B733A31303A22636F6E7465787475616C223B693A343B733A393A2264617368626F617264223B693A353B733A343A2268656C70223B693A363B733A353A22696D616765223B693A373B733A343A226C697374223B693A383B733A343A226D656E75223B693A393B733A363A226E756D626572223B693A31303B733A373A226F7074696F6E73223B693A31313B733A343A2270617468223B693A31323B733A383A227461786F6E6F6D79223B693A31333B733A353A2264626C6F67223B693A31343B733A363A22736561726368223B693A31353B733A383A2273686F7274637574223B693A31363B733A373A22746F6F6C626172223B693A31373B733A373A226F7665726C6179223B693A31383B733A383A226669656C645F7569223B693A31393B733A343A2266696C65223B693A32303B733A333A22726466223B7D733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A353A226D74696D65223B693A313431363432393438383B733A373A227061636B616765223B733A353A224F74686572223B733A333A22706870223B733A353A22352E322E34223B733A353A2266696C6573223B613A303A7B7D733A393A22626F6F747374726170223B693A303B733A363A2268696464656E223B623A313B733A383A227265717569726564223B623A313B733A31373A22646973747269627574696F6E5F6E616D65223B733A363A2244727570616C223B7D);
INSERT INTO `system` VALUES ('rcredits/rcredits.module','rcredits','module','',1,1,0,0,0x613A31303A7B733A343A226E616D65223B733A383A227243726564697473223B733A31313A226465736372697074696F6E223B733A36353A22436F7265207472616E73616374696F6E2070726F63657373696E6720666F7220746865207243726564697473206D757475616C206372656469742073797374656D223B733A373A227061636B616765223B733A383A227243726564697473223B733A343A22636F7265223B733A333A22372E78223B733A333A22706870223B733A333A22352E33223B733A353A2266696C6573223B613A393A7B693A303B733A31353A2272637265646974732E6D6F64756C65223B693A313B733A31363A2272637265646974732E696E7374616C6C223B693A323B733A32313A2272637265646974732D73657474696E67732E696E63223B693A333B733A393A227263726F6E2E696E63223B693A343B733A31373A2272637265646974732D7574696C2E696E63223B693A353B733A31323A2272637265646974732E696E63223B693A363B733A32303A2272637265646974732D6261636B656E642E696E63223B693A373B733A31353A2272637265646974732D64622E696E63223B693A383B733A31353A2261646D696E2F61646D696E2E696E63223B7D733A353A226D74696D65223B693A313431383538363032343B733A31323A22646570656E64656E63696573223B613A303A7B7D733A373A2276657273696F6E223B4E3B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('rcredits/rsmart/rsmart.module','rsmart','module','',1,0,0,0,0x613A31303A7B733A343A226E616D65223B733A33313A22724372656469747320536D6172742044657669636520496E74657266616365223B733A31313A226465736372697074696F6E223B733A34363A22536D6172742064657669636520496E7465726661636520746F207468652072437265646974732073797374656D2E223B733A373A227061636B616765223B733A383A227243726564697473223B733A343A22636F7265223B733A333A22372E78223B733A333A22706870223B733A333A22352E33223B733A31323A22646570656E64656E63696573223B613A313A7B693A303B733A383A227263726564697473223B7D733A353A2266696C6573223B613A32393A7B693A303B733A31333A2272736D6172742E6D6F64756C65223B693A313B733A31303A2272736D6172742E696E63223B693A323B733A32313A2274657374732F4964656E7469667951522E74657374223B693A333B733A31383A2274657374732F537461727475702E74657374223B693A343B733A31393A2274657374732F5472616E736163742E74657374223B693A353B733A33333A2274657374732F5472616E736163744D656D626572546F4D656D6265722E74657374223B693A363B733A31353A2274657374732F556E646F2E74657374223B693A373B733A32343A2274657374732F556E646F436F6D706C657465642E74657374223B693A383B733A32323A2274657374732F556E646F50656E64696E672E74657374223B693A393B733A32313A2274657374732F556E646F41747461636B2E74657374223B693A31303B733A32333A2274657374732F496E73756666696369656E742E74657374223B693A31313B733A31373A2274657374732F4368616E67652E74657374223B693A31323B733A31363A2274657374732F6164686F632E74657374223B693A31333B733A32313A2274657374732F556E696C61746572616C2E74657374223B693A31343B733A32373A2274657374732F4964656E74696679437573746F6D65722E74657374223B693A31353B733A31393A2274657374732F4964656E746966792E74657374223B693A31363B733A31393A2274657374732F45786368616E67652E74657374223B693A31373B733A31353A2274657374732F54696D652E74657374223B693A31383B733A31383A2274657374732F4F66666C696E652E74657374223B693A31393B733A31383A2274657374732F5370656369616C2E74657374223B693A32303B733A31363A2274657374732F4A6F696E742E74657374223B693A32313B733A31383A22746573742F45786368616E67652E74657374223B693A32323B733A31383A22746573742F4964656E746966792E74657374223B693A32333B733A31353A22746573742F4A6F696E742E74657374223B693A32343B733A31373A22746573742F4F66666C696E652E74657374223B693A32353B733A31373A22746573742F537461727475702E74657374223B693A32363B733A31343A22746573742F54696D652E74657374223B693A32373B733A31383A22746573742F5472616E736163742E74657374223B693A32383B733A31343A22746573742F556E646F2E74657374223B7D733A353A226D74696D65223B693A313436363731373232323B733A373A2276657273696F6E223B4E3B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('rcredits/rsms/rsms.module','rsms','module','',1,0,0,0,0x613A31303A7B733A343A226E616D65223B733A32323A22724372656469747320534D5320696E74657266616365223B733A31313A226465736372697074696F6E223B733A38343A22526571756573742065786368616E6765732C20696E666F726D6174696F6E2C20616E64206F74686572206F7065726174696F6E732066726F6D20796F75722063656C6C2070686F6E652C207573696E6720534D53223B733A373A227061636B616765223B733A383A227243726564697473223B733A343A22636F7265223B733A333A22372E78223B733A333A22706870223B733A333A22352E33223B733A31323A22646570656E64656E63696573223B613A313A7B693A303B733A383A227263726564697473223B7D733A353A2266696C6573223B613A32313A7B693A303B733A31323A2272736D732E696E7374616C6C223B693A313B733A31313A2272736D732E6D6F64756C65223B693A323B733A383A2272736D732E696E63223B693A333B733A31333A2272736D732D63616C6C2E696E63223B693A343B733A32303A222E2E2F72637265646974732D7574696C2E696E63223B693A353B733A31393A222E2E2F72637265646974732D6170692E696E63223B693A363B733A31303A2272736D732E7374657073223B693A373B733A32313A22676865726B696E2F746573745F646566732E706870223B693A383B733A32383A2274657374732F416262726576696174696F6E73576F726B2E74657374223B693A393B733A32363A2274657374732F45786368616E6765466F72436173682E74657374223B693A31303B733A31383A2274657374732F47657448656C702E74657374223B693A31313B733A32353A2274657374732F476574496E666F726D6174696F6E2E74657374223B693A31323B733A34363A2274657374732F4F66666572546F45786368616E67655553446F6C6C617273466F7252437265646974732E74657374223B693A31333B733A33363A2274657374732F4F70656E416E4163636F756E74466F7254686543616C6C65722E74657374223B693A31343B733A31393A2274657374732F5472616E736163742E74657374223B693A31353B733A31353A2274657374732F556E646F2E74657374223B693A31363B733A32343A2274657374732F416262726576696174696F6E732E74657374223B693A31373B733A31353A2274657374732F42616E6B2E74657374223B693A31383B733A31393A2274657374732F45786368616E67652E74657374223B693A31393B733A31353A2274657374732F48656C702E74657374223B693A32303B733A32323A2274657374732F496E666F726D6174696F6E2E74657374223B7D733A353A226D74696D65223B693A313430373037313939363B733A373A2276657273696F6E223B4E3B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('rcredits/rweb/rweb.module','rweb','module','',1,0,0,0,0x613A31303A7B733A343A226E616D65223B733A32323A2272437265646974732057656220496E74657266616365223B733A31313A226465736372697074696F6E223B733A34353A225765622042726F7773657220496E7465726661636520746F207468652072437265646974732073797374656D2E223B733A373A227061636B616765223B733A383A227243726564697473223B733A343A22636F7265223B733A333A22372E78223B733A333A22706870223B733A333A22352E33223B733A31323A22646570656E64656E63696573223B613A313A7B693A303B733A383A227263726564697473223B7D733A353A2266696C6573223B613A36343A7B693A303B733A31313A22727765622E6D6F64756C65223B693A313B733A383A22727765622E696E63223B693A323B733A31323A22727765622D7478732E696E63223B693A333B733A32323A222E2E2F61646D696E2F61646D696E2D7765622E696E63223B693A343B733A32343A222E2E2F61646D696E2F61646D696E2D666F726D732E696E63223B693A353B733A32383A2274657374732F414D656D6265724861734F7074696F6E732E74657374223B693A363B733A32363A2274657374732F45786368616E6765466F72436173682E74657374223B693A373B733A31383A2274657374732F47657448656C702E74657374223B693A383B733A32353A2274657374732F476574496E666F726D6174696F6E2E74657374223B693A393B733A34363A2274657374732F4F66666572546F45786368616E67655553446F6C6C617273466F7252437265646974732E74657374223B693A31303B733A33363A2274657374732F4F70656E416E4163636F756E74466F7254686543616C6C65722E74657374223B693A31313B733A31373A2274657374732F5369676E75702E74657374223B693A31323B733A32323A2274657374732F5472616E73616374696F6E2E74657374223B693A31333B733A31353A2274657374732F556E646F2E74657374223B693A31343B733A31393A2274657374732F5472616E736163742E74657374223B693A31353B733A31383A2274657374732F53756D6D6172792E74657374223B693A31363B733A32333A2274657374732F5472616E73616374696F6E732E74657374223B693A31373B733A32303A2274657374732F52656C6174696F6E732E74657374223B693A31383B733A31363A2274657374732F6D756C74692E74657374223B693A31393B733A32313A2274657374732F4D656D626572736869702E74657374223B693A32303B733A31383A2274657374732F5265616C5573642E74657374223B693A32313B733A32303A2274657374732F5472616E73616374522E74657374223B693A32323B733A32323A2274657374732F5472616E736163745573642E74657374223B693A32333B733A31373A2274657374732F5369676E696E2E74657374223B693A32343B733A31353A2274657374732F476966742E74657374223B693A32353B733A31353A2274657374732F466C6F772E74657374223B693A32363B733A31393A2274657374732F446F776E6C6F61642E74657374223B693A32373B733A31343A2274657374732F4765742E74657374223B693A32383B733A32373A2274657374732F496E636F6D706C6574655573645478732E74657374223B693A32393B733A31393A2274657374732F5363616E436172642E74657374223B693A33303B733A31383A2274657374732F436F6E746163742E74657374223B693A33313B733A32313A2274657374732F537461746973746963732E74657374223B693A33323B733A32303A2274657374732F436F6D6D756E6974792E74657374223B693A33333B733A31363A2274657374732F426F7865732E74657374223B693A33343B733A31353A2274657374732F42616E6B2E74657374223B693A33353B733A31373A2274657374732F4564697454782E74657374223B693A33363B733A31383A2274657374732F436F6D70616E792E74657374223B693A33373B733A32323A2274657374732F4D656D62657273686970322E74657374223B693A33383B733A31363A2274657374732F4A6F696E742E74657374223B693A33393B733A31393A2274657374732F45786368616E67652E74657374223B693A34303B733A32303A2274657374732F5265696D62757273652E74657374223B693A34313B733A31393A2274657374732F5369676E7570436F2E74657374223B693A34323B733A32323A2274657374732F507265666572656E6365732E74657374223B693A34333B733A31343A22746573742F42616E6B2E74657374223B693A34343B733A31393A22746573742F436F6D6D756E6974792E74657374223B693A34353B733A31373A22746573742F436F6D70616E792E74657374223B693A34363B733A31373A22746573742F436F6E746163742E74657374223B693A34373B733A31383A22746573742F446F776E6C6F61642E74657374223B693A34383B733A31363A22746573742F4564697454782E74657374223B693A34393B733A31383A22746573742F45786368616E67652E74657374223B693A35303B733A31343A22746573742F466C6F772E74657374223B693A35313B733A31343A22746573742F476966742E74657374223B693A35323B733A31353A22746573742F4A6F696E742E74657374223B693A35333B733A32303A22746573742F4D656D626572736869702E74657374223B693A35343B733A32313A22746573742F507265666572656E6365732E74657374223B693A35353B733A31393A22746573742F5265696D62757273652E74657374223B693A35363B733A31393A22746573742F52656C6174696F6E732E74657374223B693A35373B733A31383A22746573742F5363616E436172642E74657374223B693A35383B733A31363A22746573742F5369676E696E2E74657374223B693A35393B733A31363A22746573742F5369676E75702E74657374223B693A36303B733A31383A22746573742F5369676E7570436F2E74657374223B693A36313B733A31373A22746573742F53756D6D6172792E74657374223B693A36323B733A31383A22746573742F5472616E736163742E74657374223B693A36333B733A32323A22746573742F5472616E73616374696F6E732E74657374223B7D733A353A226D74696D65223B693A313436363731373232383B733A373A2276657273696F6E223B4E3B733A393A22626F6F747374726170223B693A303B7D);
INSERT INTO `system` VALUES ('rcredits/theme/theme.info','rcredits','theme','themes/engines/phptemplate/phptemplate.engine',1,1,0,0,0x613A31343A7B733A343A226E616D65223B733A383A227243726564697473223B733A31313A226465736372697074696F6E223B733A38303A224120766172696174696F6E206F6E20526573706F6E736976652042617274696B20287072652D72656C6561736520323031322D3038292C20666F72207468652072437265646974732053797374656D2E223B733A373A2276657273696F6E223B733A333A22312E30223B733A343A22636F7265223B733A333A22372E78223B733A31313A227374796C65736865657473223B613A323A7B733A333A22616C6C223B613A333A7B733A31343A226373732F6C61796F75742E637373223B733A32393A2272637265646974732F7468656D652F6373732F6C61796F75742E637373223B733A31333A226373732F7374796C652E637373223B733A32383A2272637265646974732F7468656D652F6373732F7374796C652E637373223B733A31343A226373732F636F6C6F72732E637373223B733A32393A2272637265646974732F7468656D652F6373732F636F6C6F72732E637373223B7D733A353A227072696E74223B613A313A7B733A31333A226373732F7072696E742E637373223B733A32383A2272637265646974732F7468656D652F6373732F7072696E742E637373223B7D7D733A373A22726567696F6E73223B613A31383A7B733A363A22686561646572223B733A363A22486561646572223B733A343A2268656C70223B733A343A2248656C70223B733A383A22706167655F746F70223B733A383A225061676520746F70223B733A31313A22706167655F626F74746F6D223B733A31313A225061676520626F74746F6D223B733A31313A22686967686C696768746564223B733A31313A22486967686C696768746564223B733A383A226163636F756E7473223B733A31363A224163636F756E742073656C6563746F72223B733A383A226665617475726564223B733A383A224665617475726564223B733A373A22636F6E74656E74223B733A373A22436F6E74656E74223B733A31333A22736964656261725F6669727374223B733A31333A2253696465626172206669727374223B733A31343A22736964656261725F7365636F6E64223B733A31343A2253696465626172207365636F6E64223B733A31343A2274726970747963685F6669727374223B733A31343A225472697074796368206669727374223B733A31353A2274726970747963685F6D6964646C65223B733A31353A225472697074796368206D6964646C65223B733A31333A2274726970747963685F6C617374223B733A31333A225472697074796368206C617374223B733A31383A22666F6F7465725F6669727374636F6C756D6E223B733A31393A22466F6F74657220666972737420636F6C756D6E223B733A31393A22666F6F7465725F7365636F6E64636F6C756D6E223B733A32303A22466F6F746572207365636F6E6420636F6C756D6E223B733A31383A22666F6F7465725F7468697264636F6C756D6E223B733A31393A22466F6F74657220746869726420636F6C756D6E223B733A31393A22666F6F7465725F666F75727468636F6C756D6E223B733A32303A22466F6F74657220666F7572746820636F6C756D6E223B733A363A22666F6F746572223B733A363A22466F6F746572223B7D733A383A2273657474696E6773223B613A313A7B733A32303A2273686F72746375745F6D6F64756C655F6C696E6B223B733A313A2230223B7D733A363A22656E67696E65223B733A31313A2270687074656D706C617465223B733A383A226665617475726573223B613A393A7B693A303B733A343A226C6F676F223B693A313B733A373A2266617669636F6E223B693A323B733A343A226E616D65223B693A333B733A363A22736C6F67616E223B693A343B733A31373A226E6F64655F757365725F70696374757265223B693A353B733A32303A22636F6D6D656E745F757365725F70696374757265223B693A363B733A32353A22636F6D6D656E745F757365725F766572696669636174696F6E223B693A373B733A393A226D61696E5F6D656E75223B693A383B733A31343A227365636F6E646172795F6D656E75223B7D733A31303A2273637265656E73686F74223B733A34303A2273697465732F616C6C2F7468656D65732F72637265646974732F73637265656E73686F742E706E67223B733A333A22706870223B733A353A22352E322E34223B733A373A2273637269707473223B613A303A7B7D733A353A226D74696D65223B693A313335343831383432363B733A31343A22726567696F6E735F68696464656E223B613A323A7B693A303B733A383A22706167655F746F70223B693A313B733A31313A22706167655F626F74746F6D223B7D7D);
INSERT INTO `system` VALUES ('themes/bartik/bartik.info','bartik','theme','themes/engines/phptemplate/phptemplate.engine',0,0,-1,0,0x613A31373A7B733A343A226E616D65223B733A363A2242617274696B223B733A31313A226465736372697074696F6E223B733A34383A224120666C657869626C652C207265636F6C6F7261626C65207468656D652077697468206D616E7920726567696F6E732E223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A31313A227374796C65736865657473223B613A323A7B733A333A22616C6C223B613A333A7B733A31343A226373732F6C61796F75742E637373223B733A32383A227468656D65732F62617274696B2F6373732F6C61796F75742E637373223B733A31333A226373732F7374796C652E637373223B733A32373A227468656D65732F62617274696B2F6373732F7374796C652E637373223B733A31343A226373732F636F6C6F72732E637373223B733A32383A227468656D65732F62617274696B2F6373732F636F6C6F72732E637373223B7D733A353A227072696E74223B613A313A7B733A31333A226373732F7072696E742E637373223B733A32373A227468656D65732F62617274696B2F6373732F7072696E742E637373223B7D7D733A373A22726567696F6E73223B613A31373A7B733A363A22686561646572223B733A363A22486561646572223B733A343A2268656C70223B733A343A2248656C70223B733A383A22706167655F746F70223B733A383A225061676520746F70223B733A31313A22706167655F626F74746F6D223B733A31313A225061676520626F74746F6D223B733A31313A22686967686C696768746564223B733A31313A22486967686C696768746564223B733A383A226665617475726564223B733A383A224665617475726564223B733A373A22636F6E74656E74223B733A373A22436F6E74656E74223B733A31333A22736964656261725F6669727374223B733A31333A2253696465626172206669727374223B733A31343A22736964656261725F7365636F6E64223B733A31343A2253696465626172207365636F6E64223B733A31343A2274726970747963685F6669727374223B733A31343A225472697074796368206669727374223B733A31353A2274726970747963685F6D6964646C65223B733A31353A225472697074796368206D6964646C65223B733A31333A2274726970747963685F6C617374223B733A31333A225472697074796368206C617374223B733A31383A22666F6F7465725F6669727374636F6C756D6E223B733A31393A22466F6F74657220666972737420636F6C756D6E223B733A31393A22666F6F7465725F7365636F6E64636F6C756D6E223B733A32303A22466F6F746572207365636F6E6420636F6C756D6E223B733A31383A22666F6F7465725F7468697264636F6C756D6E223B733A31393A22466F6F74657220746869726420636F6C756D6E223B733A31393A22666F6F7465725F666F75727468636F6C756D6E223B733A32303A22466F6F74657220666F7572746820636F6C756D6E223B733A363A22666F6F746572223B733A363A22466F6F746572223B7D733A383A2273657474696E6773223B613A313A7B733A32303A2273686F72746375745F6D6F64756C655F6C696E6B223B733A313A2230223B7D733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A363A22656E67696E65223B733A31313A2270687074656D706C617465223B733A383A226665617475726573223B613A393A7B693A303B733A343A226C6F676F223B693A313B733A373A2266617669636F6E223B693A323B733A343A226E616D65223B693A333B733A363A22736C6F67616E223B693A343B733A31373A226E6F64655F757365725F70696374757265223B693A353B733A32303A22636F6D6D656E745F757365725F70696374757265223B693A363B733A32353A22636F6D6D656E745F757365725F766572696669636174696F6E223B693A373B733A393A226D61696E5F6D656E75223B693A383B733A31343A227365636F6E646172795F6D656E75223B7D733A31303A2273637265656E73686F74223B733A32383A227468656D65732F62617274696B2F73637265656E73686F742E706E67223B733A333A22706870223B733A353A22352E322E34223B733A373A2273637269707473223B613A303A7B7D733A353A226D74696D65223B693A313431363432393438383B733A31343A22726567696F6E735F68696464656E223B613A323A7B693A303B733A383A22706167655F746F70223B693A313B733A31313A22706167655F626F74746F6D223B7D7D);
INSERT INTO `system` VALUES ('themes/garland/garland.info','garland','theme','themes/engines/phptemplate/phptemplate.engine',0,0,-1,0,0x613A31373A7B733A343A226E616D65223B733A373A224761726C616E64223B733A31313A226465736372697074696F6E223B733A3131313A2241206D756C74692D636F6C756D6E207468656D652077686963682063616E20626520636F6E6669677572656420746F206D6F6469667920636F6C6F727320616E6420737769746368206265747765656E20666978656420616E6420666C756964207769647468206C61796F7574732E223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A31313A227374796C65736865657473223B613A323A7B733A333A22616C6C223B613A313A7B733A393A227374796C652E637373223B733A32343A227468656D65732F6761726C616E642F7374796C652E637373223B7D733A353A227072696E74223B613A313A7B733A393A227072696E742E637373223B733A32343A227468656D65732F6761726C616E642F7072696E742E637373223B7D7D733A383A2273657474696E6773223B613A313A7B733A31333A226761726C616E645F7769647468223B733A353A22666C756964223B7D733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A363A22656E67696E65223B733A31313A2270687074656D706C617465223B733A373A22726567696F6E73223B613A393A7B733A31333A22736964656261725F6669727374223B733A31323A224C6566742073696465626172223B733A31343A22736964656261725F7365636F6E64223B733A31333A2252696768742073696465626172223B733A373A22636F6E74656E74223B733A373A22436F6E74656E74223B733A363A22686561646572223B733A363A22486561646572223B733A363A22666F6F746572223B733A363A22466F6F746572223B733A31313A22686967686C696768746564223B733A31313A22486967686C696768746564223B733A343A2268656C70223B733A343A2248656C70223B733A383A22706167655F746F70223B733A383A225061676520746F70223B733A31313A22706167655F626F74746F6D223B733A31313A225061676520626F74746F6D223B7D733A383A226665617475726573223B613A393A7B693A303B733A343A226C6F676F223B693A313B733A373A2266617669636F6E223B693A323B733A343A226E616D65223B693A333B733A363A22736C6F67616E223B693A343B733A31373A226E6F64655F757365725F70696374757265223B693A353B733A32303A22636F6D6D656E745F757365725F70696374757265223B693A363B733A32353A22636F6D6D656E745F757365725F766572696669636174696F6E223B693A373B733A393A226D61696E5F6D656E75223B693A383B733A31343A227365636F6E646172795F6D656E75223B7D733A31303A2273637265656E73686F74223B733A32393A227468656D65732F6761726C616E642F73637265656E73686F742E706E67223B733A333A22706870223B733A353A22352E322E34223B733A373A2273637269707473223B613A303A7B7D733A353A226D74696D65223B693A313431363432393438383B733A31343A22726567696F6E735F68696464656E223B613A323A7B693A303B733A383A22706167655F746F70223B693A313B733A31313A22706167655F626F74746F6D223B7D7D);
INSERT INTO `system` VALUES ('themes/seven/seven.info','seven','theme','themes/engines/phptemplate/phptemplate.engine',1,0,-1,0,0x613A31373A7B733A343A226E616D65223B733A353A22536576656E223B733A31313A226465736372697074696F6E223B733A36353A22412073696D706C65206F6E652D636F6C756D6E2C207461626C656C6573732C20666C7569642077696474682061646D696E697374726174696F6E207468656D652E223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A31313A227374796C65736865657473223B613A313A7B733A363A2273637265656E223B613A323A7B733A393A2272657365742E637373223B733A32323A227468656D65732F736576656E2F72657365742E637373223B733A393A227374796C652E637373223B733A32323A227468656D65732F736576656E2F7374796C652E637373223B7D7D733A383A2273657474696E6773223B613A313A7B733A32303A2273686F72746375745F6D6F64756C655F6C696E6B223B733A313A2231223B7D733A373A22726567696F6E73223B613A353A7B733A373A22636F6E74656E74223B733A373A22436F6E74656E74223B733A343A2268656C70223B733A343A2248656C70223B733A383A22706167655F746F70223B733A383A225061676520746F70223B733A31313A22706167655F626F74746F6D223B733A31313A225061676520626F74746F6D223B733A31333A22736964656261725F6669727374223B733A31333A2246697273742073696465626172223B7D733A31343A22726567696F6E735F68696464656E223B613A333A7B693A303B733A31333A22736964656261725F6669727374223B693A313B733A383A22706167655F746F70223B693A323B733A31313A22706167655F626F74746F6D223B7D733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A363A22656E67696E65223B733A31313A2270687074656D706C617465223B733A383A226665617475726573223B613A393A7B693A303B733A343A226C6F676F223B693A313B733A373A2266617669636F6E223B693A323B733A343A226E616D65223B693A333B733A363A22736C6F67616E223B693A343B733A31373A226E6F64655F757365725F70696374757265223B693A353B733A32303A22636F6D6D656E745F757365725F70696374757265223B693A363B733A32353A22636F6D6D656E745F757365725F766572696669636174696F6E223B693A373B733A393A226D61696E5F6D656E75223B693A383B733A31343A227365636F6E646172795F6D656E75223B7D733A31303A2273637265656E73686F74223B733A32373A227468656D65732F736576656E2F73637265656E73686F742E706E67223B733A333A22706870223B733A353A22352E322E34223B733A373A2273637269707473223B613A303A7B7D733A353A226D74696D65223B693A313431363432393438383B7D);
INSERT INTO `system` VALUES ('themes/stark/stark.info','stark','theme','themes/engines/phptemplate/phptemplate.engine',0,0,-1,0,0x613A31363A7B733A343A226E616D65223B733A353A22537461726B223B733A31313A226465736372697074696F6E223B733A3230383A2254686973207468656D652064656D6F6E737472617465732044727570616C27732064656661756C742048544D4C206D61726B757020616E6420435353207374796C65732E20546F206C6561726E20686F7720746F206275696C6420796F7572206F776E207468656D6520616E64206F766572726964652044727570616C27732064656661756C7420636F64652C2073656520746865203C6120687265663D22687474703A2F2F64727570616C2E6F72672F7468656D652D6775696465223E5468656D696E672047756964653C2F613E2E223B733A373A227061636B616765223B733A343A22436F7265223B733A373A2276657273696F6E223B733A343A22372E3334223B733A343A22636F7265223B733A333A22372E78223B733A31313A227374796C65736865657473223B613A313A7B733A333A22616C6C223B613A313A7B733A31303A226C61796F75742E637373223B733A32333A227468656D65732F737461726B2F6C61796F75742E637373223B7D7D733A373A2270726F6A656374223B733A363A2264727570616C223B733A393A22646174657374616D70223B733A31303A2231343136343239343838223B733A363A22656E67696E65223B733A31313A2270687074656D706C617465223B733A373A22726567696F6E73223B613A393A7B733A31333A22736964656261725F6669727374223B733A31323A224C6566742073696465626172223B733A31343A22736964656261725F7365636F6E64223B733A31333A2252696768742073696465626172223B733A373A22636F6E74656E74223B733A373A22436F6E74656E74223B733A363A22686561646572223B733A363A22486561646572223B733A363A22666F6F746572223B733A363A22466F6F746572223B733A31313A22686967686C696768746564223B733A31313A22486967686C696768746564223B733A343A2268656C70223B733A343A2248656C70223B733A383A22706167655F746F70223B733A383A225061676520746F70223B733A31313A22706167655F626F74746F6D223B733A31313A225061676520626F74746F6D223B7D733A383A226665617475726573223B613A393A7B693A303B733A343A226C6F676F223B693A313B733A373A2266617669636F6E223B693A323B733A343A226E616D65223B693A333B733A363A22736C6F67616E223B693A343B733A31373A226E6F64655F757365725F70696374757265223B693A353B733A32303A22636F6D6D656E745F757365725F70696374757265223B693A363B733A32353A22636F6D6D656E745F757365725F766572696669636174696F6E223B693A373B733A393A226D61696E5F6D656E75223B693A383B733A31343A227365636F6E646172795F6D656E75223B7D733A31303A2273637265656E73686F74223B733A32373A227468656D65732F737461726B2F73637265656E73686F742E706E67223B733A333A22706870223B733A353A22352E322E34223B733A373A2273637269707473223B613A303A7B7D733A353A226D74696D65223B693A313431363432393438383B733A31343A22726567696F6E735F68696464656E223B613A323A7B693A303B733A383A22706167655F746F70223B693A313B733A31313A22706167655F626F74746F6D223B7D7D);
/*!40000 ALTER TABLE `system` ENABLE KEYS */;
UNLOCK TABLES;

LOCK TABLES `tx_cats` WRITE;
/*!40000 ALTER TABLE `tx_cats` DISABLE KEYS */;
INSERT INTO `tx_cats` VALUES (-1,'Loan or CG-to-CG transfer','Loan to or from a CG Loan Fund OR a CG-to-CG transfer (internal only)',0,1,NULL,'CG2CG');
INSERT INTO `tx_cats` VALUES (100,'I: Billable Expense Income','',265,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (200,'I: Consulting and Services','Consulting Income',155,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (300,'I: Donations','',156,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (400,'I: Donations: Ads','',157,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (500,'I: Donations: Company Donations','',233,1,NULL,'D-COMPANY');
INSERT INTO `tx_cats` VALUES (600,'I: Donations: Crumbs Donations','percentage of payments received through Common Good',280,1,NULL,'D-CRUMB');
INSERT INTO `tx_cats` VALUES (800,'I: Donations: Grants','',159,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (900,'I: Donations: One-time Donations','usually from non-members (always, before FY2019)',234,1,NULL,'D-ONCE');
INSERT INTO `tx_cats` VALUES (1000,'I: Donations: Regular Donations','recurring monthly, quarterly, or yearly (until FY2019 included ALL member donations)',196,1,NULL,'D-REGULAR');
INSERT INTO `tx_cats` VALUES (1100,'I: Donations: Roundup Donations','automatic contribution of rounded up payment change',279,1,NULL,'D-ROUNDUP');
INSERT INTO `tx_cats` VALUES (1150,'I: Donations: Sponsored','donations to sponsored projects',194,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (1200,'I: Donations: Sponsored: Sponsored Donations','passed through to an organization we fiscally sponsor',318,1,NULL,'D-FBO');
INSERT INTO `tx_cats` VALUES (1250,'I: Donations: Sponsored: Sponsored Stepups','stepup donations to sponsored projects',315,1,NULL,'D-FBO-STEPUP');
INSERT INTO `tx_cats` VALUES (1300,'I: Donations: Sponsored Donations: Fiscal Sponsorship Fees','part of Sponsored Donations not to be spent on program costs',322,1,NULL,'FS-FEE');
INSERT INTO `tx_cats` VALUES (1400,'I: Donations: Stepup Donations and Tips','',311,1,NULL,'D-STEPUP');
INSERT INTO `tx_cats` VALUES (1700,'I: Gross Sales','Gross Sales',161,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (1800,'I: Income Uncertainty','Rewards, Inflation Adjustment, and Shared Rewards',222,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (1900,'I: Investment Income','',295,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (2000,'I: Investment Income: Bank Interest','Bank Interest',244,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (2100,'I: Investment Income: Program-related Investment Income','Investment Income in keeping with our mission',162,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (2200,'I: Markup','',258,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (2300,'I: Sales of Product Income','',262,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (2400,'I: Uncategorized Income','Income not categorized elsewh',188,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (2500,'E: Ask William','',267,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (2600,'E: Bad Debt','Bad Debt Expense',164,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (2700,'E: Change in Risk Assessment','Change in expected value of investments or contingent grants (in or out)',293,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (2800,'E: Cost of Goods Sold','',263,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (2900,'E: Depreciation','',281,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (3000,'E: Depreciation: Depreciation - FY2014 Furniture','',286,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (3100,'E: Depreciation: Depreciation - FY2016 Equipment','',283,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (3200,'E: Depreciation: Depreciation - FY2017 Equipment','',284,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (3300,'E: Depreciation: Depreciation - FY2018 Equipment','',285,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (3400,'E: Depreciation: Depreciation FY2014 - Equipment','',282,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (3500,'E: Equipment','',165,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (3600,'E: Equipment: CG POS equipment','',202,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (3700,'E: Equipment: Computer Hardware & Software','',190,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (3800,'E: Equipment: Repairs','Repairs',182,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (3900,'E: Equipment: Resources (Books, etc.)','',183,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (4000,'E: Event Costs','Food, lodging, equipment etc.',166,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (4100,'E: Event Costs: Consumables','',207,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (4200,'E: Event Costs: Equipment Rental','',246,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (4300,'E: Event Costs: Event Fees & Space Rental','',169,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (4400,'E: Fees','',167,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (4500,'E: Fees: Government Fees','',170,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (4600,'E: Fees: Interest Expense','Interest Expense',174,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (4700,'E: Fees: Legal Fees','',171,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (4800,'E: Fees: Tax Penalties and Interest','',240,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (4900,'E: Fees: Transaction Fees','fees for credit card transactions, wires, etc.',168,1,NULL,'TX-FEE');
INSERT INTO `tx_cats` VALUES (5000,'E: Fees: Transaction Fees: Reimbursement of Transaction Fees','mostly from sponsored organizations',323,1,NULL,'TX-FEE-BACK');
INSERT INTO `tx_cats` VALUES (5100,'E: Grants','',172,1,NULL,'CG-GRANT-ORG');
INSERT INTO `tx_cats` VALUES (5200,'E: Information Services','Dues and Subscription Expense',173,1,NULL,'INFO-SVC');
INSERT INTO `tx_cats` VALUES (5300,'E: Information Services: Websites','',185,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (5400,'E: Marketing - Advanced','Promotional Expenses',198,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (5500,'E: Marketing - Advanced: Entertainment','Entertainment',256,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (5600,'E: Marketing - Advanced: Networking Fees and Dues','',253,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (5700,'E: Marketing - Simple','',230,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (5800,'E: Marketing - Simple: Advertising','Advertising',251,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (5900,'E: Marketing - Simple: Member Support','',303,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (6000,'E: Marketing - Simple: Postage / Shipping','Postage and Delivery Expense',255,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (6100,'E: Marketing - Simple: Printing','Printing and Repro. Expense',248,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (6200,'E: Marketing - Simple: Promotional Materials','',245,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (6300,'E: Miscellaneous','Miscellaneous',175,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (6400,'E: Miscellaneous: CG Account Adjustments','',203,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (6500,'E: Miscellaneous: Reconciliation Discrepancies','Discrepancies between bank statements and company records',226,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (6600,'E: Office','',176,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (6700,'E: Office: Communication Services','internet, phone, zoom, otter.ai, etc.',242,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (6800,'E: Office: Insurance','Insurance',252,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (6900,'E: Office: Rent','',189,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (7000,'E: Office: Supplies','Supplies',177,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (7100,'E: Office: Utilities','Water, Gas, Electric',178,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (7200,'E: Opening Balance Equity','Opening balances during setup',145,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (7300,'E: Payroll Expenses','Payroll expenses',209,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (7400,'E: Payroll Expenses: Payroll Fee','',232,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (7500,'E: Payroll Expenses: Payroll Taxes','',237,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (7600,'E: Payroll Expenses: Payroll Wages','',239,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (7700,'E: Professional Fees','',179,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (7800,'E: Professional Fees: Accounting Services','',201,1,NULL,'ACCOUNTING');
INSERT INTO `tx_cats` VALUES (7900,'E: Professional Fees: Ad Hoc Consultants','',180,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (8000,'E: Professional Fees: Computer Services','',220,1,NULL,'COMPUTER');
INSERT INTO `tx_cats` VALUES (8100,'E: Professional Fees: Honoraria','appreciative discretionary compensation for miscellaneous services -- includes \"thank you\" gifts to',219,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (8200,'E: Professional Fees: Staff Consultants','Professional Fees',181,1,NULL,'CONTRACTOR');
INSERT INTO `tx_cats` VALUES (8300,'E: Professional Fees: Staff Development','',224,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (8400,'E: Purchases','',261,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (8500,'E: Reconciliation Discrepancies','',266,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (8600,'E: Reconciliation Discrepancies-1','',296,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (8700,'E: Retained Earnings','Undistributed earnings of the',144,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (8800,'E: Sponsored Project Expenses','',314,1,NULL,'FIRST-FS-EXPENSE');
INSERT INTO `tx_cats` VALUES (8810,'E: Sponsored Project Expenses: Advertising and Promotion','',347,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (8820,'E: Sponsored Project Expenses: Conferences, Conventions, and Meetings','',352,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (8830,'E: Sponsored Project Expenses: Fees for Services','',340,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (8840,'E: Sponsored Project Expenses: Fees for Services: Accounting','',343,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (8850,'E: Sponsored Project Expenses: Fees for Services: Fundraising','',344,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (8860,'E: Sponsored Project Expenses: Fees for Services: Investment Management','',345,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (8870,'E: Sponsored Project Expenses: Fees for Services: Legal','',341,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (8880,'E: Sponsored Project Expenses: Fees for Services: Management','',342,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (8890,'E: Sponsored Project Expenses: Fees for Services: Staff Development','',346,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (8895,'E: Sponsored Project Expenses: Government Fees','',355,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (8900,'E: Sponsored Project Expenses: Grants and Direct Support to Individuals','Direct support to eligible members for food costs',316,1,NULL,'GRANT-PERSON');
INSERT INTO `tx_cats` VALUES (8905,'E: Sponsored Project Expenses: Grants to Domestic Organizations','',338,1,NULL,'GRANT-ORG');
INSERT INTO `tx_cats` VALUES (8910,'E: Sponsored Project Expenses: Grants to Foreign Organizations and Individuals','',339,1,NULL,'GRANT-FOREIGN');
INSERT INTO `tx_cats` VALUES (8920,'E: Sponsored Project Expenses: Information Technology','',349,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (8930,'E: Sponsored Project Expenses: Insurance','',354,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (8940,'E: Sponsored Project Expenses: Interest Expense','',353,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (8945,'E: Sponsored Project Expenses: Office Expenses','',348,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (8950,'E: Sponsored Project Expenses: Project Equipment','equipment used for a sponsored project',337,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (9000,'E: Sponsored Project Expenses: Real Estate Purchases and Occupancy','rent or acquisition of real estate',325,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (9050,'E: Sponsored Project Expenses: Royalties','',350,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (9100,'E: Sponsored Project Expenses: Payroll','',317,1,NULL,'FBO-LABOR');
INSERT INTO `tx_cats` VALUES (9125,'E: Sponsored Project Expenses: Payroll Taxes','',356,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (9150,'E: Sponsored Project Expenses: Transaction Fees','fees for credit card transactions, wires, etc.',328,1,NULL,'FBO-TX-FEE');
INSERT INTO `tx_cats` VALUES (9175,'E: Sponsored Project Expenses: Travel','',351,1,NULL,'LAST-FS-EXPENSE');
INSERT INTO `tx_cats` VALUES (9200,'E: Travel','Car & Truck',184,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (9300,'E: Unapplied Cash Bill Payment Expense','',297,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (9400,'E: Uncategorized Expense','',259,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (9500,'E: _Accrued Int','Accrued Interest',186,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (9600,'E: _IntExp','Investment Interest Exp',187,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (9700,'A: *Accounts Receivable','Unpaid or unapplied customer',193,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (9800,'A: 457 Escrow Asset','',290,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (9900,'A: 457 Escrow Asset: 457 Investments','money is in shared capital cooperative',302,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (10000,'A: Accounts Receivable','',152,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (10100,'A: CC Processor','credit card processor, like PayPal',150,1,NULL,'PROCESSOR');
INSERT INTO `tx_cats` VALUES (10150,'A: CG 457 Escrow','',223,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (10200,'A: CG Account ..AAB','Common Good\'s own Common Good Credits Account',195,1,NULL,'NEWAAB');
INSERT INTO `tx_cats` VALUES (10300,'A: Inventory Asset','',264,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (10400,'A: Investments','',211,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (10500,'A: Investments: Artisan Beverage Coop Investment','',277,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (10600,'A: Investments: Boston Community Loan Fund','',273,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (10700,'A: Investments: CG Western MA Region Investments','investments / loans made by the \"Region\"',312,1,NULL,'INVEST');
INSERT INTO `tx_cats` VALUES (10800,'A: Investments: Co-op Power Investment','',278,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (10900,'A: Investments: Equity Trust Investment','',274,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (11000,'A: Investments: NH Community Loan Fund','',269,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (11100,'A: Investments: Northeast Biodiesel Investment','',268,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (11150,'A: Investments: Other Loans','',329,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (11200,'A: Investments: PVGrows Investment','',275,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (11300,'A: Investments: x Loan Loss Reserve - CG Western MA','',313,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (11400,'A: Investments: x Loan Loss Reserve - Community Funds (5%)','',288,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (11500,'A: Investments: x Loan Loss Reserve - Northeast Biodiesel (50%)','',287,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (11600,'A: Office Equipment','major equipment such as computers (depreciate at 20% per year)',214,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (11700,'A: Office Furniture','',212,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (11800,'A: Old Checking ..2275','at Citizens Bank. Old corporate account #909302151 at Greenfield Cooperative Bank. Closed October 31',146,1,NULL,'OLD-BANK');
INSERT INTO `tx_cats` VALUES (11900,'A: Operations ..8571','Brattleboro S&L account #500718571',307,1,NULL,'OPERATIONS');
INSERT INTO `tx_cats` VALUES (12000,'A: Petty Cash','',151,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (12100,'A: Sponsored','Projects sponsored by CG',319,1,NULL,'SPONSORED');
INSERT INTO `tx_cats` VALUES (12200,'A: Sponsored: CG Western MA Region ..AAA','The server\'s \"regional\" account',310,1,NULL,'!NEWAAA');
INSERT INTO `tx_cats` VALUES (12250,'A: Sponsored: Dollar Pool','',330,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (12275,'A: Sponsored: Dollar Pool: CC Processor Sponsored','part of CC account that is part of the Dollar Pool',205,1,NULL,'FBO-PROCESSOR');
INSERT INTO `tx_cats` VALUES (12300,'A: Sponsored: Dollar Pool ..8598','Brattleboro S&L account #500718598',306,1,NULL,'POOL-A');
INSERT INTO `tx_cats` VALUES (12400,'A: Sponsored: Dollar Pool ..8598: MSB Escrow for Dollar Pool','BS&L asked to hold $50k in escrow',327,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (12500,'A: Sponsored: Food Fund ..AZV','Common Good\'s fund for food subsidies',309,1,NULL,'NEWAZV');
INSERT INTO `tx_cats` VALUES (12600,'A: Sponsored: Kibilio ..BTY','FBO Kibilio (fiscal sponsorship)',320,1,NULL,'NEWBTY');
INSERT INTO `tx_cats` VALUES (12700,'A: Sponsored: RJ Brooklyn ..AUN','FBO Racial Justice Brooklyn (fiscal sponsorship)',321,1,NULL,'NYAAUN');
INSERT INTO `tx_cats` VALUES (12800,'A: Uncategorized Asset','',260,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (12900,'L: 457 Deferred Retirement Pay','',299,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (13000,'L: 457 Deferred Retirement Pay: 457 Deferred Retirement Pay - WS','',218,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (13100,'L: Accounts Payable','Unpaid or unapplied vendor bills or credits',231,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (13200,'L: Capital One CC','',153,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (13300,'L: Community Funding Guarantees FY2017-18 (10%)','We promised Common Good Greenfield, to back their first two year\'s funding ($10 + $18k).',289,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (13400,'L: Contingent Compensation','compensation CGF owes to some former contractors',192,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (13500,'L: Dollar Pool Liability','Amt in CG Dollar Pool we owe to CG Communities (their members, collectively) -- net member transfers',204,1,NULL,'POOL');
INSERT INTO `tx_cats` VALUES (13600,'L: EIDL Advance','',305,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (13700,'L: Equipment Deposits','',225,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (13800,'L: Long Term Loans','',229,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (13900,'L: Long Term Loans: Advance Investments','for CGBank Project',243,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (14000,'L: Long Term Loans: CG Greenfield Loan for Co-op Power','',292,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (14100,'L: Long Term Loans: Forgivable Loans','',254,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (14150,'L: Long Term Loans: Loan Fund Liability','money lent to Common Good loan funds, still owed back',374,1,NULL,'LOANFUND');
INSERT INTO `tx_cats` VALUES (14200,'L: Long Term Loans: Loan from Sally Willoughby','at 2.5% ($250 due 2/1/2020, $10,250 due 2/1/2021)',300,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (14300,'L: Long Term Loans: S2BE checking accounts','money owed to participants',250,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (14400,'L: Negative Balance Risk of Non-Repayment (10%)','How much of the total of all negative balances would people fail to repay.',308,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (14500,'L: Payroll Liabilities','Unpaid payroll liabilities. Amounts withheld or accrued, but not yet paid',210,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (14600,'L: Payroll Liabilities: 941 Taxes Liability','',236,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (14700,'L: Payroll Liabilities: State SUTA/UI Tax Liability','',238,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (14800,'L: Payroll Liabilities: State Withholding Tax Liability','',217,1,NULL,NULL);
INSERT INTO `tx_cats` VALUES (14900,'L: Payroll Liabilities: To Be Paid In CG Credits','',215,1,NULL,'LABOR');
/*!40000 ALTER TABLE `tx_cats` ENABLE KEYS */;
UNLOCK TABLES;

LOCK TABLES `users` WRITE;
/*!40000 ALTER TABLE `users` DISABLE KEYS */;
INSERT INTO `users` VALUES (0,NULL,NULL,NULL,0,0,0,0,NULL,0,0,0,NULL,0,NULL,NULL,NULL,NULL,NULL,0,NULL,0,0.00000000,0.00000000,NULL,0,0,0,0,0,NULL,0.00,0.00,0.00,NULL,0.000,0.00,0,NULL,0.000,0.00,0.00,0.00,NULL,0,0,NULL,NULL,NULL,0,NULL,NULL);
/*!40000 ALTER TABLE `users` ENABLE KEYS */;
UNLOCK TABLES;

LOCK TABLES `variable` WRITE;
/*!40000 ALTER TABLE `variable` DISABLE KEYS */;
INSERT INTO `variable` VALUES ('menu_expanded',0x613A303A7B7D);
INSERT INTO `variable` VALUES ('menu_masks',0x613A31303A7B693A303B693A3132353B693A313B693A3132313B693A323B693A36333B693A333B693A36323B693A343B693A36303B693A353B693A33313B693A363B693A31353B693A373B693A373B693A383B693A333B693A393B693A313B7D);
INSERT INTO `variable` VALUES ('up',0x623A313B);
/*!40000 ALTER TABLE `variable` ENABLE KEYS */;
UNLOCK TABLES;

LOCK TABLES `zip3` WRITE;
/*!40000 ALTER TABLE `zip3` DISABLE KEYS */;
INSERT INTO `zip3` VALUES ('005','Mid-Island','NY');
INSERT INTO `zip3` VALUES ('006','San Juan','PR');
INSERT INTO `zip3` VALUES ('007','San Juan','PR');
INSERT INTO `zip3` VALUES ('008','San Juan','PR');
INSERT INTO `zip3` VALUES ('009','San Juan','PR');
INSERT INTO `zip3` VALUES ('010','Springfield','MA');
INSERT INTO `zip3` VALUES ('011','Springfield','MA');
INSERT INTO `zip3` VALUES ('012','Pittsfield','MA');
INSERT INTO `zip3` VALUES ('013','Springfield','MA');
INSERT INTO `zip3` VALUES ('014','Central','MA');
INSERT INTO `zip3` VALUES ('015','Central','MA');
INSERT INTO `zip3` VALUES ('016','Worcester','MA');
INSERT INTO `zip3` VALUES ('017','Central','MA');
INSERT INTO `zip3` VALUES ('018','Middlesex-Esx','MA');
INSERT INTO `zip3` VALUES ('019','Middlesex-Esx','MA');
INSERT INTO `zip3` VALUES ('020','Brockton','MA');
INSERT INTO `zip3` VALUES ('021','Boston','MA');
INSERT INTO `zip3` VALUES ('022','Boston','MA');
INSERT INTO `zip3` VALUES ('023','Brockton','MA');
INSERT INTO `zip3` VALUES ('024','Northwest Bos','MA');
INSERT INTO `zip3` VALUES ('025','Cape Cod','MA');
INSERT INTO `zip3` VALUES ('026','Cape Cod','MA');
INSERT INTO `zip3` VALUES ('027','Providence','RI');
INSERT INTO `zip3` VALUES ('028','Providence','RI');
INSERT INTO `zip3` VALUES ('029','Providence','RI');
INSERT INTO `zip3` VALUES ('030','Manchester','NH');
INSERT INTO `zip3` VALUES ('031','Manchester','NH');
INSERT INTO `zip3` VALUES ('032','Manchester','NH');
INSERT INTO `zip3` VALUES ('033','Concord','NH');
INSERT INTO `zip3` VALUES ('034','Manchester','NH');
INSERT INTO `zip3` VALUES ('035','White Riv Jct','VT');
INSERT INTO `zip3` VALUES ('036','White Riv Jct','VT');
INSERT INTO `zip3` VALUES ('037','White Riv Jct','VT');
INSERT INTO `zip3` VALUES ('038','Portsmouth','NH');
INSERT INTO `zip3` VALUES ('039','Portsmouth','NH');
INSERT INTO `zip3` VALUES ('040','Portland','ME');
INSERT INTO `zip3` VALUES ('041','Portland','ME');
INSERT INTO `zip3` VALUES ('042','Portland','ME');
INSERT INTO `zip3` VALUES ('043','Portland','ME');
INSERT INTO `zip3` VALUES ('044','Bangor','ME');
INSERT INTO `zip3` VALUES ('045','Portland','ME');
INSERT INTO `zip3` VALUES ('046','Bangor','ME');
INSERT INTO `zip3` VALUES ('047','Bangor','ME');
INSERT INTO `zip3` VALUES ('048','Portland','ME');
INSERT INTO `zip3` VALUES ('049','Bangor','ME');
INSERT INTO `zip3` VALUES ('050','White Riv Jct','VT');
INSERT INTO `zip3` VALUES ('051','White Riv Jct','VT');
INSERT INTO `zip3` VALUES ('052','White Riv Jct','VT');
INSERT INTO `zip3` VALUES ('053','White Riv Jct','VT');
INSERT INTO `zip3` VALUES ('054','Burlington','VT');
INSERT INTO `zip3` VALUES ('055','Middlesex-Esx','MA');
INSERT INTO `zip3` VALUES ('056','Burlington','VT');
INSERT INTO `zip3` VALUES ('057','White Riv Jct','VT');
INSERT INTO `zip3` VALUES ('058','White Riv Jct','VT');
INSERT INTO `zip3` VALUES ('059','White Riv Jct','VT');
INSERT INTO `zip3` VALUES ('060','Hartford','CT');
INSERT INTO `zip3` VALUES ('061','Hartford','CT');
INSERT INTO `zip3` VALUES ('062','Hartford','CT');
INSERT INTO `zip3` VALUES ('063','Southern','CT');
INSERT INTO `zip3` VALUES ('064','Southern','CT');
INSERT INTO `zip3` VALUES ('065','New Haven','CT');
INSERT INTO `zip3` VALUES ('066','Bridgeport','CT');
INSERT INTO `zip3` VALUES ('067','Waterbury','CT');
INSERT INTO `zip3` VALUES ('068','Stamford','CT');
INSERT INTO `zip3` VALUES ('069','Stamford','CT');
INSERT INTO `zip3` VALUES ('070','Newark','NJ');
INSERT INTO `zip3` VALUES ('071','Newark','NJ');
INSERT INTO `zip3` VALUES ('072','Elizabeth','NJ');
INSERT INTO `zip3` VALUES ('073','Jersey City','NJ');
INSERT INTO `zip3` VALUES ('074','Paterson','NJ');
INSERT INTO `zip3` VALUES ('075','Paterson','NJ');
INSERT INTO `zip3` VALUES ('076','Hackensack','NJ');
INSERT INTO `zip3` VALUES ('077','Monmouth','NJ');
INSERT INTO `zip3` VALUES ('078','West Jersey','NJ');
INSERT INTO `zip3` VALUES ('079','West Jersey','NJ');
INSERT INTO `zip3` VALUES ('080','South Jersey','NJ');
INSERT INTO `zip3` VALUES ('081','Camden','NJ');
INSERT INTO `zip3` VALUES ('082','South Jersey','NJ');
INSERT INTO `zip3` VALUES ('083','South Jersey','NJ');
INSERT INTO `zip3` VALUES ('084','Atlantic City','NJ');
INSERT INTO `zip3` VALUES ('085','Trenton','NJ');
INSERT INTO `zip3` VALUES ('086','Trenton','NJ');
INSERT INTO `zip3` VALUES ('087','Monmouth','NJ');
INSERT INTO `zip3` VALUES ('088','Kilmer','NJ');
INSERT INTO `zip3` VALUES ('089','New Brunswick','NJ');
INSERT INTO `zip3` VALUES ('090','Apo','AE');
INSERT INTO `zip3` VALUES ('091','Apo','AE');
INSERT INTO `zip3` VALUES ('092','Apo','AE');
INSERT INTO `zip3` VALUES ('093','Apo','AE');
INSERT INTO `zip3` VALUES ('094','Apo/Fpo','AE');
INSERT INTO `zip3` VALUES ('095','Fpo','AE');
INSERT INTO `zip3` VALUES ('096','Apo/Fpo','AE');
INSERT INTO `zip3` VALUES ('097','Apo/Fpo','AE');
INSERT INTO `zip3` VALUES ('098','Apo/Fpo','AE');
INSERT INTO `zip3` VALUES ('099','Apo/Fpo','AE');
INSERT INTO `zip3` VALUES ('100','New York','NY');
INSERT INTO `zip3` VALUES ('101','New York','NY');
INSERT INTO `zip3` VALUES ('102','New York','NY');
INSERT INTO `zip3` VALUES ('103','Staten Island','NY');
INSERT INTO `zip3` VALUES ('104','Bronx','NY');
INSERT INTO `zip3` VALUES ('105','Westchester','NY');
INSERT INTO `zip3` VALUES ('106','White Plains','NY');
INSERT INTO `zip3` VALUES ('107','Yonkers','NY');
INSERT INTO `zip3` VALUES ('108','New Rochelle','NY');
INSERT INTO `zip3` VALUES ('109','Westchester','NY');
INSERT INTO `zip3` VALUES ('110','Queens','NY');
INSERT INTO `zip3` VALUES ('111','Long Island City','NY');
INSERT INTO `zip3` VALUES ('112','Brooklyn','NY');
INSERT INTO `zip3` VALUES ('113','Flushing','NY');
INSERT INTO `zip3` VALUES ('114','Jamaica','NY');
INSERT INTO `zip3` VALUES ('115','Western Nassau','NY');
INSERT INTO `zip3` VALUES ('116','Far Rockaway','NY');
INSERT INTO `zip3` VALUES ('117','Mid-Island','NY');
INSERT INTO `zip3` VALUES ('118','Hicksville','NY');
INSERT INTO `zip3` VALUES ('119','Mid-Island','NY');
INSERT INTO `zip3` VALUES ('120','Albany','NY');
INSERT INTO `zip3` VALUES ('121','Albany','NY');
INSERT INTO `zip3` VALUES ('122','Albany','NY');
INSERT INTO `zip3` VALUES ('123','Schenectady','NY');
INSERT INTO `zip3` VALUES ('124','Mid-Hudson','NY');
INSERT INTO `zip3` VALUES ('125','Mid-Hudson','NY');
INSERT INTO `zip3` VALUES ('126','Poughkeepsie','NY');
INSERT INTO `zip3` VALUES ('127','Mid-Hudson','NY');
INSERT INTO `zip3` VALUES ('128','Glens Falls','NY');
INSERT INTO `zip3` VALUES ('129','Plattsburgh','NY');
INSERT INTO `zip3` VALUES ('130','Syracuse','NY');
INSERT INTO `zip3` VALUES ('131','Syracuse','NY');
INSERT INTO `zip3` VALUES ('132','Syracuse','NY');
INSERT INTO `zip3` VALUES ('133','Utica','NY');
INSERT INTO `zip3` VALUES ('134','Utica','NY');
INSERT INTO `zip3` VALUES ('135','Utica','NY');
INSERT INTO `zip3` VALUES ('136','Watertown','NY');
INSERT INTO `zip3` VALUES ('137','Binghamton','NY');
INSERT INTO `zip3` VALUES ('138','Binghamton','NY');
INSERT INTO `zip3` VALUES ('139','Binghamton','NY');
INSERT INTO `zip3` VALUES ('140','Buffalo','NY');
INSERT INTO `zip3` VALUES ('141','Buffalo','NY');
INSERT INTO `zip3` VALUES ('142','Buffalo','NY');
INSERT INTO `zip3` VALUES ('143','Niagara Falls','NY');
INSERT INTO `zip3` VALUES ('144','Rochester','NY');
INSERT INTO `zip3` VALUES ('145','Rochester','NY');
INSERT INTO `zip3` VALUES ('146','Rochester','NY');
INSERT INTO `zip3` VALUES ('147','Jamestown','NY');
INSERT INTO `zip3` VALUES ('148','Elmira','NY');
INSERT INTO `zip3` VALUES ('149','Elmira','NY');
INSERT INTO `zip3` VALUES ('150','Pittsburgh','PA');
INSERT INTO `zip3` VALUES ('151','Pittsburgh','PA');
INSERT INTO `zip3` VALUES ('152','Pittsburgh','PA');
INSERT INTO `zip3` VALUES ('153','Pittsburgh','PA');
INSERT INTO `zip3` VALUES ('154','Pittsburgh','PA');
INSERT INTO `zip3` VALUES ('155','Johnstown','PA');
INSERT INTO `zip3` VALUES ('156','Greensburg','PA');
INSERT INTO `zip3` VALUES ('157','Johnstown','PA');
INSERT INTO `zip3` VALUES ('158','Du Bois','PA');
INSERT INTO `zip3` VALUES ('159','Johnstown','PA');
INSERT INTO `zip3` VALUES ('160','New Castle','PA');
INSERT INTO `zip3` VALUES ('161','New Castle','PA');
INSERT INTO `zip3` VALUES ('162','New Castle','PA');
INSERT INTO `zip3` VALUES ('163','Oil City','PA');
INSERT INTO `zip3` VALUES ('164','Erie','PA');
INSERT INTO `zip3` VALUES ('165','Erie','PA');
INSERT INTO `zip3` VALUES ('166','Altoona','PA');
INSERT INTO `zip3` VALUES ('167','Bradford','PA');
INSERT INTO `zip3` VALUES ('168','Altoona','PA');
INSERT INTO `zip3` VALUES ('169','Williamsport','PA');
INSERT INTO `zip3` VALUES ('170','Harrisburg','PA');
INSERT INTO `zip3` VALUES ('171','Harrisburg','PA');
INSERT INTO `zip3` VALUES ('172','Harrisburg','PA');
INSERT INTO `zip3` VALUES ('173','Lancaster','PA');
INSERT INTO `zip3` VALUES ('174','York','PA');
INSERT INTO `zip3` VALUES ('175','Lancaster','PA');
INSERT INTO `zip3` VALUES ('176','Lancaster','PA');
INSERT INTO `zip3` VALUES ('177','Williamsport','PA');
INSERT INTO `zip3` VALUES ('178','Harrisburg','PA');
INSERT INTO `zip3` VALUES ('179','Reading','PA');
INSERT INTO `zip3` VALUES ('180','Lehigh Valley','PA');
INSERT INTO `zip3` VALUES ('181','Allentown','PA');
INSERT INTO `zip3` VALUES ('182','Wilkes Barre','PA');
INSERT INTO `zip3` VALUES ('183','Lehigh Valley','PA');
INSERT INTO `zip3` VALUES ('184','Scranton','PA');
INSERT INTO `zip3` VALUES ('185','Scranton','PA');
INSERT INTO `zip3` VALUES ('186','Wilkes Barre','PA');
INSERT INTO `zip3` VALUES ('187','Wilkes Barre','PA');
INSERT INTO `zip3` VALUES ('188','Scranton','PA');
INSERT INTO `zip3` VALUES ('189','Southeastern','PA');
INSERT INTO `zip3` VALUES ('190','Philadelphia','PA');
INSERT INTO `zip3` VALUES ('191','Philadelphia','PA');
INSERT INTO `zip3` VALUES ('192','Philadelphia','PA');
INSERT INTO `zip3` VALUES ('193','Southeastern','PA');
INSERT INTO `zip3` VALUES ('194','Southeastern','PA');
INSERT INTO `zip3` VALUES ('195','Reading','PA');
INSERT INTO `zip3` VALUES ('196','Reading','PA');
INSERT INTO `zip3` VALUES ('197','Wilmington','DE');
INSERT INTO `zip3` VALUES ('198','Wilmington','DE');
INSERT INTO `zip3` VALUES ('199','Wilmington','DE');
INSERT INTO `zip3` VALUES ('200','Washington','DC');
INSERT INTO `zip3` VALUES ('201','Dulles','VA');
INSERT INTO `zip3` VALUES ('202','Washington','DC');
INSERT INTO `zip3` VALUES ('203','Washington','DC');
INSERT INTO `zip3` VALUES ('204','Washington','DC');
INSERT INTO `zip3` VALUES ('205','Washington','DC');
INSERT INTO `zip3` VALUES ('206','Southern Md','MD');
INSERT INTO `zip3` VALUES ('207','Southern Md','MD');
INSERT INTO `zip3` VALUES ('208','Suburban Md','MD');
INSERT INTO `zip3` VALUES ('209','Silver Spring','MD');
INSERT INTO `zip3` VALUES ('210','Linthicum','MD');
INSERT INTO `zip3` VALUES ('211','Linthicum','MD');
INSERT INTO `zip3` VALUES ('212','Baltimore','MD');
INSERT INTO `zip3` VALUES ('214','Annapolis','MD');
INSERT INTO `zip3` VALUES ('215','Cumberland','MD');
INSERT INTO `zip3` VALUES ('216','Eastern Shore','MD');
INSERT INTO `zip3` VALUES ('217','Frederick','MD');
INSERT INTO `zip3` VALUES ('218','Salisbury','MD');
INSERT INTO `zip3` VALUES ('219','Baltimore','MD');
INSERT INTO `zip3` VALUES ('220','Northern Va','VA');
INSERT INTO `zip3` VALUES ('221','Northern Va','VA');
INSERT INTO `zip3` VALUES ('222','Arlington','VA');
INSERT INTO `zip3` VALUES ('223','Alexandria','VA');
INSERT INTO `zip3` VALUES ('224','Richmond','VA');
INSERT INTO `zip3` VALUES ('225','Richmond','VA');
INSERT INTO `zip3` VALUES ('226','Winchester','VA');
INSERT INTO `zip3` VALUES ('227','Culpeper','VA');
INSERT INTO `zip3` VALUES ('228','Charlottesvle','VA');
INSERT INTO `zip3` VALUES ('229','Charlottesvle','VA');
INSERT INTO `zip3` VALUES ('230','Richmond','VA');
INSERT INTO `zip3` VALUES ('231','Richmond','VA');
INSERT INTO `zip3` VALUES ('232','Richmond','VA');
INSERT INTO `zip3` VALUES ('233','Norfolk','VA');
INSERT INTO `zip3` VALUES ('234','Norfolk','VA');
INSERT INTO `zip3` VALUES ('235','Norfolk','VA');
INSERT INTO `zip3` VALUES ('236','Norfolk','VA');
INSERT INTO `zip3` VALUES ('237','Portsmouth','VA');
INSERT INTO `zip3` VALUES ('238','Richmond','VA');
INSERT INTO `zip3` VALUES ('239','Farmville','VA');
INSERT INTO `zip3` VALUES ('240','Roanoke','VA');
INSERT INTO `zip3` VALUES ('241','Roanoke','VA');
INSERT INTO `zip3` VALUES ('242','Bristol','VA');
INSERT INTO `zip3` VALUES ('243','Roanoke','VA');
INSERT INTO `zip3` VALUES ('244','Charlottesvle','VA');
INSERT INTO `zip3` VALUES ('245','Lynchburg','VA');
INSERT INTO `zip3` VALUES ('246','Bluefield','WV');
INSERT INTO `zip3` VALUES ('247','Bluefield','WV');
INSERT INTO `zip3` VALUES ('248','Bluefield','WV');
INSERT INTO `zip3` VALUES ('249','Lewisburg','WV');
INSERT INTO `zip3` VALUES ('250','Charleston','WV');
INSERT INTO `zip3` VALUES ('251','Charleston','WV');
INSERT INTO `zip3` VALUES ('252','Charleston','WV');
INSERT INTO `zip3` VALUES ('253','Charleston','WV');
INSERT INTO `zip3` VALUES ('254','Martinsburg','WV');
INSERT INTO `zip3` VALUES ('255','Huntington','WV');
INSERT INTO `zip3` VALUES ('256','Huntington','WV');
INSERT INTO `zip3` VALUES ('257','Huntington','WV');
INSERT INTO `zip3` VALUES ('258','Beckley','WV');
INSERT INTO `zip3` VALUES ('259','Beckley','WV');
INSERT INTO `zip3` VALUES ('260','Wheeling','WV');
INSERT INTO `zip3` VALUES ('261','Parkersburg','WV');
INSERT INTO `zip3` VALUES ('262','Clarksburg','WV');
INSERT INTO `zip3` VALUES ('263','Clarksburg','WV');
INSERT INTO `zip3` VALUES ('264','Clarksburg','WV');
INSERT INTO `zip3` VALUES ('265','Clarksburg','WV');
INSERT INTO `zip3` VALUES ('266','Gassaway','WV');
INSERT INTO `zip3` VALUES ('267','Cumberland','MD');
INSERT INTO `zip3` VALUES ('268','Petersburg','WV');
INSERT INTO `zip3` VALUES ('270','Greensboro','NC');
INSERT INTO `zip3` VALUES ('271','Winston-Salem','NC');
INSERT INTO `zip3` VALUES ('272','Greensboro','NC');
INSERT INTO `zip3` VALUES ('273','Greensboro','NC');
INSERT INTO `zip3` VALUES ('274','Greensboro','NC');
INSERT INTO `zip3` VALUES ('275','Raleigh','NC');
INSERT INTO `zip3` VALUES ('276','Raleigh','NC');
INSERT INTO `zip3` VALUES ('277','Durham','NC');
INSERT INTO `zip3` VALUES ('278','Rocky Mount','NC');
INSERT INTO `zip3` VALUES ('279','Rocky Mount','NC');
INSERT INTO `zip3` VALUES ('280','Charlotte','NC');
INSERT INTO `zip3` VALUES ('281','Charlotte','NC');
INSERT INTO `zip3` VALUES ('282','Charlotte','NC');
INSERT INTO `zip3` VALUES ('283','Fayetteville','NC');
INSERT INTO `zip3` VALUES ('284','Fayetteville','NC');
INSERT INTO `zip3` VALUES ('285','Kinston','NC');
INSERT INTO `zip3` VALUES ('286','Hickory','NC');
INSERT INTO `zip3` VALUES ('287','Asheville','NC');
INSERT INTO `zip3` VALUES ('288','Asheville','NC');
INSERT INTO `zip3` VALUES ('289','Asheville','NC');
INSERT INTO `zip3` VALUES ('290','Columbia','SC');
INSERT INTO `zip3` VALUES ('291','Columbia','SC');
INSERT INTO `zip3` VALUES ('292','Columbia','SC');
INSERT INTO `zip3` VALUES ('293','Greenville','SC');
INSERT INTO `zip3` VALUES ('294','Charleston','SC');
INSERT INTO `zip3` VALUES ('295','Florence','SC');
INSERT INTO `zip3` VALUES ('296','Greenville','SC');
INSERT INTO `zip3` VALUES ('297','Charlotte','NC');
INSERT INTO `zip3` VALUES ('298','Augusta','GA');
INSERT INTO `zip3` VALUES ('299','Savannah','GA');
INSERT INTO `zip3` VALUES ('300','North Metro','GA');
INSERT INTO `zip3` VALUES ('301','North Metro','GA');
INSERT INTO `zip3` VALUES ('302','Atlanta','GA');
INSERT INTO `zip3` VALUES ('303','Atlanta','GA');
INSERT INTO `zip3` VALUES ('304','Swainsboro','GA');
INSERT INTO `zip3` VALUES ('305','Athens','GA');
INSERT INTO `zip3` VALUES ('306','Athens','GA');
INSERT INTO `zip3` VALUES ('307','Chattanooga','TN');
INSERT INTO `zip3` VALUES ('308','Augusta','GA');
INSERT INTO `zip3` VALUES ('309','Augusta','GA');
INSERT INTO `zip3` VALUES ('310','Macon','GA');
INSERT INTO `zip3` VALUES ('311','Atlanta','GA');
INSERT INTO `zip3` VALUES ('312','Macon','GA');
INSERT INTO `zip3` VALUES ('313','Savannah','GA');
INSERT INTO `zip3` VALUES ('314','Savannah','GA');
INSERT INTO `zip3` VALUES ('315','Waycross','GA');
INSERT INTO `zip3` VALUES ('316','Valdosta','GA');
INSERT INTO `zip3` VALUES ('317','Albany','GA');
INSERT INTO `zip3` VALUES ('318','Columbus','GA');
INSERT INTO `zip3` VALUES ('319','Columbus','GA');
INSERT INTO `zip3` VALUES ('320','Jacksonville','FL');
INSERT INTO `zip3` VALUES ('321','Daytona Beach','FL');
INSERT INTO `zip3` VALUES ('322','Jacksonville','FL');
INSERT INTO `zip3` VALUES ('323','Tallahassee','FL');
INSERT INTO `zip3` VALUES ('324','Panama City','FL');
INSERT INTO `zip3` VALUES ('325','Pensacola','FL');
INSERT INTO `zip3` VALUES ('326','Gainesville','FL');
INSERT INTO `zip3` VALUES ('327','Mid-Florida','FL');
INSERT INTO `zip3` VALUES ('328','Orlando','FL');
INSERT INTO `zip3` VALUES ('329','Orlando','FL');
INSERT INTO `zip3` VALUES ('330','South Florida','FL');
INSERT INTO `zip3` VALUES ('331','Miami','FL');
INSERT INTO `zip3` VALUES ('332','Miami','FL');
INSERT INTO `zip3` VALUES ('333','Ft Lauderdale','FL');
INSERT INTO `zip3` VALUES ('334','West Palm Bch','FL');
INSERT INTO `zip3` VALUES ('335','Tampa','FL');
INSERT INTO `zip3` VALUES ('336','Tampa','FL');
INSERT INTO `zip3` VALUES ('337','St Petersburg','FL');
INSERT INTO `zip3` VALUES ('338','Lakeland','FL');
INSERT INTO `zip3` VALUES ('339','Ft Myers','FL');
INSERT INTO `zip3` VALUES ('340','Apo/Fpo','AA');
INSERT INTO `zip3` VALUES ('341','Ft Myers','FL');
INSERT INTO `zip3` VALUES ('342','Manasota','FL');
INSERT INTO `zip3` VALUES ('344','Gainesville','FL');
INSERT INTO `zip3` VALUES ('346','Tampa','FL');
INSERT INTO `zip3` VALUES ('347','Orlando','FL');
INSERT INTO `zip3` VALUES ('349','West Palm Bch','FL');
INSERT INTO `zip3` VALUES ('350','Birmingham','AL');
INSERT INTO `zip3` VALUES ('351','Birmingham','AL');
INSERT INTO `zip3` VALUES ('352','Birmingham','AL');
INSERT INTO `zip3` VALUES ('354','Tuscaloosa','AL');
INSERT INTO `zip3` VALUES ('355','Birmingham','AL');
INSERT INTO `zip3` VALUES ('356','Huntsville','AL');
INSERT INTO `zip3` VALUES ('357','Huntsville','AL');
INSERT INTO `zip3` VALUES ('358','Huntsville','AL');
INSERT INTO `zip3` VALUES ('359','Birmingham','AL');
INSERT INTO `zip3` VALUES ('360','Montgomery','AL');
INSERT INTO `zip3` VALUES ('361','Montgomery','AL');
INSERT INTO `zip3` VALUES ('362','Anniston','AL');
INSERT INTO `zip3` VALUES ('363','Dothan','AL');
INSERT INTO `zip3` VALUES ('364','Evergreen','AL');
INSERT INTO `zip3` VALUES ('365','Mobile','AL');
INSERT INTO `zip3` VALUES ('366','Mobile','AL');
INSERT INTO `zip3` VALUES ('367','Montgomery','AL');
INSERT INTO `zip3` VALUES ('368','Montgomery','AL');
INSERT INTO `zip3` VALUES ('369','Meridian','MS');
INSERT INTO `zip3` VALUES ('370','Nashville','TN');
INSERT INTO `zip3` VALUES ('371','Nashville','TN');
INSERT INTO `zip3` VALUES ('372','Nashville','TN');
INSERT INTO `zip3` VALUES ('373','Chattanooga','TN');
INSERT INTO `zip3` VALUES ('374','Chattanooga','TN');
INSERT INTO `zip3` VALUES ('375','Memphis','TN');
INSERT INTO `zip3` VALUES ('376','Johnson City','TN');
INSERT INTO `zip3` VALUES ('377','Knoxville','TN');
INSERT INTO `zip3` VALUES ('378','Knoxville','TN');
INSERT INTO `zip3` VALUES ('379','Knoxville','TN');
INSERT INTO `zip3` VALUES ('380','Memphis','TN');
INSERT INTO `zip3` VALUES ('381','Memphis','TN');
INSERT INTO `zip3` VALUES ('382','Mckenzie','TN');
INSERT INTO `zip3` VALUES ('383','Jackson','TN');
INSERT INTO `zip3` VALUES ('384','Columbia','TN');
INSERT INTO `zip3` VALUES ('385','Cookeville','TN');
INSERT INTO `zip3` VALUES ('386','Memphis','TN');
INSERT INTO `zip3` VALUES ('387','Greenville','MS');
INSERT INTO `zip3` VALUES ('388','Tupelo','MS');
INSERT INTO `zip3` VALUES ('389','Grenada','MS');
INSERT INTO `zip3` VALUES ('390','Jackson','MS');
INSERT INTO `zip3` VALUES ('391','Jackson','MS');
INSERT INTO `zip3` VALUES ('392','Jackson','MS');
INSERT INTO `zip3` VALUES ('393','Meridian','MS');
INSERT INTO `zip3` VALUES ('394','Hattiesburg','MS');
INSERT INTO `zip3` VALUES ('395','Gulfport','MS');
INSERT INTO `zip3` VALUES ('396','Mccomb','MS');
INSERT INTO `zip3` VALUES ('397','Columbus','MS');
INSERT INTO `zip3` VALUES ('398','Albany','GA');
INSERT INTO `zip3` VALUES ('399','Atlanta','GA');
INSERT INTO `zip3` VALUES ('400','Louisville','KY');
INSERT INTO `zip3` VALUES ('401','Louisville','KY');
INSERT INTO `zip3` VALUES ('402','Louisville','KY');
INSERT INTO `zip3` VALUES ('403','Lexington','KY');
INSERT INTO `zip3` VALUES ('404','Lexington','KY');
INSERT INTO `zip3` VALUES ('405','Lexington','KY');
INSERT INTO `zip3` VALUES ('406','Frankfort','KY');
INSERT INTO `zip3` VALUES ('407','London','KY');
INSERT INTO `zip3` VALUES ('408','London','KY');
INSERT INTO `zip3` VALUES ('409','London','KY');
INSERT INTO `zip3` VALUES ('410','Cincinnati','OH');
INSERT INTO `zip3` VALUES ('411','Ashland','KY');
INSERT INTO `zip3` VALUES ('412','Ashland','KY');
INSERT INTO `zip3` VALUES ('413','Campton','KY');
INSERT INTO `zip3` VALUES ('414','Campton','KY');
INSERT INTO `zip3` VALUES ('415','Pikeville','KY');
INSERT INTO `zip3` VALUES ('416','Pikeville','KY');
INSERT INTO `zip3` VALUES ('417','Hazard','KY');
INSERT INTO `zip3` VALUES ('418','Hazard','KY');
INSERT INTO `zip3` VALUES ('420','Paducah','KY');
INSERT INTO `zip3` VALUES ('421','Bowling Green','KY');
INSERT INTO `zip3` VALUES ('422','Bowling Green','KY');
INSERT INTO `zip3` VALUES ('423','Owensboro','KY');
INSERT INTO `zip3` VALUES ('424','Evansville','IN');
INSERT INTO `zip3` VALUES ('425','Somerset','KY');
INSERT INTO `zip3` VALUES ('426','Somerset','KY');
INSERT INTO `zip3` VALUES ('427','Elizabethtown','KY');
INSERT INTO `zip3` VALUES ('430','Columbus','OH');
INSERT INTO `zip3` VALUES ('431','Columbus','OH');
INSERT INTO `zip3` VALUES ('432','Columbus','OH');
INSERT INTO `zip3` VALUES ('433','Columbus','OH');
INSERT INTO `zip3` VALUES ('434','Toledo','OH');
INSERT INTO `zip3` VALUES ('435','Toledo','OH');
INSERT INTO `zip3` VALUES ('436','Toledo','OH');
INSERT INTO `zip3` VALUES ('437','Zanesville','OH');
INSERT INTO `zip3` VALUES ('438','Zanesville','OH');
INSERT INTO `zip3` VALUES ('439','Steubenville','OH');
INSERT INTO `zip3` VALUES ('440','Cleveland','OH');
INSERT INTO `zip3` VALUES ('441','Cleveland','OH');
INSERT INTO `zip3` VALUES ('442','Akron','OH');
INSERT INTO `zip3` VALUES ('443','Akron','OH');
INSERT INTO `zip3` VALUES ('444','Youngstown','OH');
INSERT INTO `zip3` VALUES ('445','Youngstown','OH');
INSERT INTO `zip3` VALUES ('446','Canton','OH');
INSERT INTO `zip3` VALUES ('447','Canton','OH');
INSERT INTO `zip3` VALUES ('448','Mansfield','OH');
INSERT INTO `zip3` VALUES ('449','Mansfield','OH');
INSERT INTO `zip3` VALUES ('450','Cincinnati','OH');
INSERT INTO `zip3` VALUES ('451','Cincinnati','OH');
INSERT INTO `zip3` VALUES ('452','Cincinnati','OH');
INSERT INTO `zip3` VALUES ('453','Dayton','OH');
INSERT INTO `zip3` VALUES ('454','Dayton','OH');
INSERT INTO `zip3` VALUES ('455','Springfield','OH');
INSERT INTO `zip3` VALUES ('456','Chillicothe','OH');
INSERT INTO `zip3` VALUES ('457','Athens','OH');
INSERT INTO `zip3` VALUES ('458','Lima','OH');
INSERT INTO `zip3` VALUES ('459','Cincinnati','OH');
INSERT INTO `zip3` VALUES ('460','Indianapolis','IN');
INSERT INTO `zip3` VALUES ('461','Indianapolis','IN');
INSERT INTO `zip3` VALUES ('462','Indianapolis','IN');
INSERT INTO `zip3` VALUES ('463','Gary','IN');
INSERT INTO `zip3` VALUES ('464','Gary','IN');
INSERT INTO `zip3` VALUES ('465','South Bend','IN');
INSERT INTO `zip3` VALUES ('466','South Bend','IN');
INSERT INTO `zip3` VALUES ('467','Fort Wayne','IN');
INSERT INTO `zip3` VALUES ('468','Fort Wayne','IN');
INSERT INTO `zip3` VALUES ('469','Kokomo','IN');
INSERT INTO `zip3` VALUES ('470','Cincinnati','OH');
INSERT INTO `zip3` VALUES ('471','Louisville','KY');
INSERT INTO `zip3` VALUES ('472','Columbus','IN');
INSERT INTO `zip3` VALUES ('473','Muncie','IN');
INSERT INTO `zip3` VALUES ('474','Bloomington','IN');
INSERT INTO `zip3` VALUES ('475','Terre Haute','IN');
INSERT INTO `zip3` VALUES ('476','Evansville','IN');
INSERT INTO `zip3` VALUES ('477','Evansville','IN');
INSERT INTO `zip3` VALUES ('478','Terre Haute','IN');
INSERT INTO `zip3` VALUES ('479','Lafayette','IN');
INSERT INTO `zip3` VALUES ('480','Royal Oak','MI');
INSERT INTO `zip3` VALUES ('481','Detroit','MI');
INSERT INTO `zip3` VALUES ('482','Detroit','MI');
INSERT INTO `zip3` VALUES ('483','Royal Oak','MI');
INSERT INTO `zip3` VALUES ('484','Flint','MI');
INSERT INTO `zip3` VALUES ('485','Flint','MI');
INSERT INTO `zip3` VALUES ('486','Saginaw','MI');
INSERT INTO `zip3` VALUES ('487','Saginaw','MI');
INSERT INTO `zip3` VALUES ('488','Lansing','MI');
INSERT INTO `zip3` VALUES ('489','Lansing','MI');
INSERT INTO `zip3` VALUES ('490','Kalamazoo','MI');
INSERT INTO `zip3` VALUES ('491','Kalamazoo','MI');
INSERT INTO `zip3` VALUES ('492','Jackson','MI');
INSERT INTO `zip3` VALUES ('493','Grand Rapids','MI');
INSERT INTO `zip3` VALUES ('494','Grand Rapids','MI');
INSERT INTO `zip3` VALUES ('495','Grand Rapids','MI');
INSERT INTO `zip3` VALUES ('496','Traverse City','MI');
INSERT INTO `zip3` VALUES ('497','Gaylord','MI');
INSERT INTO `zip3` VALUES ('498','Iron Mountain','MI');
INSERT INTO `zip3` VALUES ('499','Iron Mountain','MI');
INSERT INTO `zip3` VALUES ('500','Des Moines','IA');
INSERT INTO `zip3` VALUES ('501','Des Moines','IA');
INSERT INTO `zip3` VALUES ('502','Des Moines','IA');
INSERT INTO `zip3` VALUES ('503','Des Moines','IA');
INSERT INTO `zip3` VALUES ('504','Waterloo','IA');
INSERT INTO `zip3` VALUES ('505','Fort Dodge','IA');
INSERT INTO `zip3` VALUES ('506','Waterloo','IA');
INSERT INTO `zip3` VALUES ('507','Waterloo','IA');
INSERT INTO `zip3` VALUES ('508','Creston','IA');
INSERT INTO `zip3` VALUES ('509','Des Moines','IA');
INSERT INTO `zip3` VALUES ('510','Sioux City','IA');
INSERT INTO `zip3` VALUES ('511','Sioux City','IA');
INSERT INTO `zip3` VALUES ('512','Sioux City','IA');
INSERT INTO `zip3` VALUES ('513','Sioux City','IA');
INSERT INTO `zip3` VALUES ('514','Carroll','IA');
INSERT INTO `zip3` VALUES ('515','Omaha','NE');
INSERT INTO `zip3` VALUES ('516','Omaha','NE');
INSERT INTO `zip3` VALUES ('520','Dubuque','IA');
INSERT INTO `zip3` VALUES ('521','Decorah','IA');
INSERT INTO `zip3` VALUES ('522','Cedar Rapids','IA');
INSERT INTO `zip3` VALUES ('523','Cedar Rapids','IA');
INSERT INTO `zip3` VALUES ('524','Cedar Rapids','IA');
INSERT INTO `zip3` VALUES ('525','Des Moines','IA');
INSERT INTO `zip3` VALUES ('526','Burlington','IA');
INSERT INTO `zip3` VALUES ('527','Quad Cities','IL');
INSERT INTO `zip3` VALUES ('528','Davenport','IA');
INSERT INTO `zip3` VALUES ('530','Milwaukee','WI');
INSERT INTO `zip3` VALUES ('531','Milwaukee','WI');
INSERT INTO `zip3` VALUES ('532','Milwaukee','WI');
INSERT INTO `zip3` VALUES ('534','Racine','WI');
INSERT INTO `zip3` VALUES ('535','Madison','WI');
INSERT INTO `zip3` VALUES ('537','Madison','WI');
INSERT INTO `zip3` VALUES ('538','Madison','WI');
INSERT INTO `zip3` VALUES ('539','Portage','WI');
INSERT INTO `zip3` VALUES ('540','St Paul','MN');
INSERT INTO `zip3` VALUES ('541','Green Bay','WI');
INSERT INTO `zip3` VALUES ('542','Green Bay','WI');
INSERT INTO `zip3` VALUES ('543','Green Bay','WI');
INSERT INTO `zip3` VALUES ('544','Wausau','WI');
INSERT INTO `zip3` VALUES ('545','Rhinelander','WI');
INSERT INTO `zip3` VALUES ('546','La Crosse','WI');
INSERT INTO `zip3` VALUES ('547','Eau Claire','WI');
INSERT INTO `zip3` VALUES ('548','Spooner','WI');
INSERT INTO `zip3` VALUES ('549','Oshkosh','WI');
INSERT INTO `zip3` VALUES ('550','St Paul','MN');
INSERT INTO `zip3` VALUES ('551','St Paul','MN');
INSERT INTO `zip3` VALUES ('553','Minneapolis','MN');
INSERT INTO `zip3` VALUES ('554','Minneapolis','MN');
INSERT INTO `zip3` VALUES ('555','Minneapolis','MN');
INSERT INTO `zip3` VALUES ('556','Duluth','MN');
INSERT INTO `zip3` VALUES ('557','Duluth','MN');
INSERT INTO `zip3` VALUES ('558','Duluth','MN');
INSERT INTO `zip3` VALUES ('559','Rochester','MN');
INSERT INTO `zip3` VALUES ('560','Mankato','MN');
INSERT INTO `zip3` VALUES ('561','Mankato','MN');
INSERT INTO `zip3` VALUES ('562','Willmar','MN');
INSERT INTO `zip3` VALUES ('563','St Cloud','MN');
INSERT INTO `zip3` VALUES ('564','Brainerd','MN');
INSERT INTO `zip3` VALUES ('565','Detroit Lakes','MN');
INSERT INTO `zip3` VALUES ('566','Bemidji','MN');
INSERT INTO `zip3` VALUES ('567','Grand Forks','ND');
INSERT INTO `zip3` VALUES ('570','Sioux Falls','SD');
INSERT INTO `zip3` VALUES ('571','Sioux Falls','SD');
INSERT INTO `zip3` VALUES ('572','Dakota Central','SD');
INSERT INTO `zip3` VALUES ('573','Dakota Central','SD');
INSERT INTO `zip3` VALUES ('574','Aberdeen','SD');
INSERT INTO `zip3` VALUES ('575','Pierre','SD');
INSERT INTO `zip3` VALUES ('576','Mobridge','SD');
INSERT INTO `zip3` VALUES ('577','Rapid City','SD');
INSERT INTO `zip3` VALUES ('580','Fargo','ND');
INSERT INTO `zip3` VALUES ('581','Fargo','ND');
INSERT INTO `zip3` VALUES ('582','Grand Forks','ND');
INSERT INTO `zip3` VALUES ('583','Devils Lake','ND');
INSERT INTO `zip3` VALUES ('584','Jamestown','ND');
INSERT INTO `zip3` VALUES ('585','Bismarck','ND');
INSERT INTO `zip3` VALUES ('586','Bismarck','ND');
INSERT INTO `zip3` VALUES ('587','Minot','ND');
INSERT INTO `zip3` VALUES ('588','Williston','ND');
INSERT INTO `zip3` VALUES ('590','Billings','MT');
INSERT INTO `zip3` VALUES ('591','Billings','MT');
INSERT INTO `zip3` VALUES ('592','Wolf Point','MT');
INSERT INTO `zip3` VALUES ('593','Miles City','MT');
INSERT INTO `zip3` VALUES ('594','Great Falls','MT');
INSERT INTO `zip3` VALUES ('595','Havre','MT');
INSERT INTO `zip3` VALUES ('596','Helena','MT');
INSERT INTO `zip3` VALUES ('597','Butte','MT');
INSERT INTO `zip3` VALUES ('598','Missoula','MT');
INSERT INTO `zip3` VALUES ('599','Kalispell','MT');
INSERT INTO `zip3` VALUES ('600','Palatine','IL');
INSERT INTO `zip3` VALUES ('601','Carol Stream','IL');
INSERT INTO `zip3` VALUES ('602','Evanston','IL');
INSERT INTO `zip3` VALUES ('603','Oak Park','IL');
INSERT INTO `zip3` VALUES ('604','S Suburban','IL');
INSERT INTO `zip3` VALUES ('605','Fox Valley','IL');
INSERT INTO `zip3` VALUES ('606','Chicago','IL');
INSERT INTO `zip3` VALUES ('607','Chicago','IL');
INSERT INTO `zip3` VALUES ('608','Chicago','IL');
INSERT INTO `zip3` VALUES ('609','Kankakee','IL');
INSERT INTO `zip3` VALUES ('610','Rockford','IL');
INSERT INTO `zip3` VALUES ('611','Rockford','IL');
INSERT INTO `zip3` VALUES ('612','Quad Cities','IL');
INSERT INTO `zip3` VALUES ('613','La Salle','IL');
INSERT INTO `zip3` VALUES ('614','Galesburg','IL');
INSERT INTO `zip3` VALUES ('615','Peoria','IL');
INSERT INTO `zip3` VALUES ('616','Peoria','IL');
INSERT INTO `zip3` VALUES ('617','Bloomington','IL');
INSERT INTO `zip3` VALUES ('618','Champaign','IL');
INSERT INTO `zip3` VALUES ('619','Champaign','IL');
INSERT INTO `zip3` VALUES ('620','St Louis','MO');
INSERT INTO `zip3` VALUES ('622','St Louis','MO');
INSERT INTO `zip3` VALUES ('623','Quincy','IL');
INSERT INTO `zip3` VALUES ('624','Effingham','IL');
INSERT INTO `zip3` VALUES ('625','Springfield','IL');
INSERT INTO `zip3` VALUES ('626','Springfield','IL');
INSERT INTO `zip3` VALUES ('627','Springfield','IL');
INSERT INTO `zip3` VALUES ('628','Centralia','IL');
INSERT INTO `zip3` VALUES ('629','Carbondale','IL');
INSERT INTO `zip3` VALUES ('630','St Louis','MO');
INSERT INTO `zip3` VALUES ('631','St Louis','MO');
INSERT INTO `zip3` VALUES ('633','St Louis','MO');
INSERT INTO `zip3` VALUES ('634','Quincy','IL');
INSERT INTO `zip3` VALUES ('635','Quincy','IL');
INSERT INTO `zip3` VALUES ('636','Cape Girardeau','MO');
INSERT INTO `zip3` VALUES ('637','Cape Girardeau','MO');
INSERT INTO `zip3` VALUES ('638','Cape Girardeau','MO');
INSERT INTO `zip3` VALUES ('639','Cape Girardeau','MO');
INSERT INTO `zip3` VALUES ('640','Kansas City','MO');
INSERT INTO `zip3` VALUES ('641','Kansas City','MO');
INSERT INTO `zip3` VALUES ('644','St Joseph','MO');
INSERT INTO `zip3` VALUES ('645','St Joseph','MO');
INSERT INTO `zip3` VALUES ('646','Chillicothe','MO');
INSERT INTO `zip3` VALUES ('647','Harrisonville','MO');
INSERT INTO `zip3` VALUES ('648','Springfield','MO');
INSERT INTO `zip3` VALUES ('649','Kansas City','MO');
INSERT INTO `zip3` VALUES ('650','Mid-Missouri','MO');
INSERT INTO `zip3` VALUES ('651','Mid-Missouri','MO');
INSERT INTO `zip3` VALUES ('652','Mid-Missouri','MO');
INSERT INTO `zip3` VALUES ('653','Mid-Missouri','MO');
INSERT INTO `zip3` VALUES ('654','Springfield','MO');
INSERT INTO `zip3` VALUES ('655','Springfield','MO');
INSERT INTO `zip3` VALUES ('656','Springfield','MO');
INSERT INTO `zip3` VALUES ('657','Springfield','MO');
INSERT INTO `zip3` VALUES ('658','Springfield','MO');
INSERT INTO `zip3` VALUES ('660','Kansas City','KS');
INSERT INTO `zip3` VALUES ('661','Kansas City','KS');
INSERT INTO `zip3` VALUES ('662','Kansas City','KS');
INSERT INTO `zip3` VALUES ('664','Topeka','KS');
INSERT INTO `zip3` VALUES ('665','Topeka','KS');
INSERT INTO `zip3` VALUES ('666','Topeka','KS');
INSERT INTO `zip3` VALUES ('667','Ft Scott','KS');
INSERT INTO `zip3` VALUES ('668','Topeka','KS');
INSERT INTO `zip3` VALUES ('669','Salina','KS');
INSERT INTO `zip3` VALUES ('670','Wichita','KS');
INSERT INTO `zip3` VALUES ('671','Wichita','KS');
INSERT INTO `zip3` VALUES ('672','Wichita','KS');
INSERT INTO `zip3` VALUES ('673','Independence','KS');
INSERT INTO `zip3` VALUES ('674','Salina','KS');
INSERT INTO `zip3` VALUES ('675','Hutchinson','KS');
INSERT INTO `zip3` VALUES ('676','Hays','KS');
INSERT INTO `zip3` VALUES ('677','Colby','KS');
INSERT INTO `zip3` VALUES ('678','Dodge City','KS');
INSERT INTO `zip3` VALUES ('679','Liberal','KS');
INSERT INTO `zip3` VALUES ('680','Omaha','NE');
INSERT INTO `zip3` VALUES ('681','Omaha','NE');
INSERT INTO `zip3` VALUES ('683','Lincoln','NE');
INSERT INTO `zip3` VALUES ('684','Lincoln','NE');
INSERT INTO `zip3` VALUES ('685','Lincoln','NE');
INSERT INTO `zip3` VALUES ('686','Norfolk','NE');
INSERT INTO `zip3` VALUES ('687','Norfolk','NE');
INSERT INTO `zip3` VALUES ('688','Grand Island','NE');
INSERT INTO `zip3` VALUES ('689','Grand Island','NE');
INSERT INTO `zip3` VALUES ('690','Mc Cook','NE');
INSERT INTO `zip3` VALUES ('691','North Platte','NE');
INSERT INTO `zip3` VALUES ('692','Valentine','NE');
INSERT INTO `zip3` VALUES ('693','Alliance','NE');
INSERT INTO `zip3` VALUES ('700','New Orleans','LA');
INSERT INTO `zip3` VALUES ('701','New Orleans','LA');
INSERT INTO `zip3` VALUES ('703','Houma','LA');
INSERT INTO `zip3` VALUES ('704','Mandeville','LA');
INSERT INTO `zip3` VALUES ('705','Lafayette','LA');
INSERT INTO `zip3` VALUES ('706','Lake Charles','LA');
INSERT INTO `zip3` VALUES ('707','Baton Rouge','LA');
INSERT INTO `zip3` VALUES ('708','Baton Rouge','LA');
INSERT INTO `zip3` VALUES ('710','Shreveport','LA');
INSERT INTO `zip3` VALUES ('711','Shreveport','LA');
INSERT INTO `zip3` VALUES ('712','Monroe','LA');
INSERT INTO `zip3` VALUES ('713','Alexandria','LA');
INSERT INTO `zip3` VALUES ('714','Alexandria','LA');
INSERT INTO `zip3` VALUES ('716','Pine Bluff','AR');
INSERT INTO `zip3` VALUES ('717','Camden','AR');
INSERT INTO `zip3` VALUES ('718','Texarkana','AR');
INSERT INTO `zip3` VALUES ('719','Hot Springs Ntl Pk','AR');
INSERT INTO `zip3` VALUES ('720','Little Rock','AR');
INSERT INTO `zip3` VALUES ('721','Little Rock','AR');
INSERT INTO `zip3` VALUES ('722','Little Rock','AR');
INSERT INTO `zip3` VALUES ('723','Memphis','TN');
INSERT INTO `zip3` VALUES ('724','Ne Arkansas','AR');
INSERT INTO `zip3` VALUES ('725','Batesville','AR');
INSERT INTO `zip3` VALUES ('726','Harrison','AR');
INSERT INTO `zip3` VALUES ('727','Nw Arkansas','AR');
INSERT INTO `zip3` VALUES ('728','Russellville','AR');
INSERT INTO `zip3` VALUES ('729','Fort Smith','AR');
INSERT INTO `zip3` VALUES ('730','Oklahoma City','OK');
INSERT INTO `zip3` VALUES ('731','Oklahoma City','OK');
INSERT INTO `zip3` VALUES ('733','Austin','TX');
INSERT INTO `zip3` VALUES ('734','Ardmore','OK');
INSERT INTO `zip3` VALUES ('735','Lawton','OK');
INSERT INTO `zip3` VALUES ('736','Clinton','OK');
INSERT INTO `zip3` VALUES ('737','Enid','OK');
INSERT INTO `zip3` VALUES ('738','Woodward','OK');
INSERT INTO `zip3` VALUES ('739','Liberal','KS');
INSERT INTO `zip3` VALUES ('740','Tulsa','OK');
INSERT INTO `zip3` VALUES ('741','Tulsa','OK');
INSERT INTO `zip3` VALUES ('743','Tulsa','OK');
INSERT INTO `zip3` VALUES ('744','Muskogee','OK');
INSERT INTO `zip3` VALUES ('745','Mcalester','OK');
INSERT INTO `zip3` VALUES ('746','Ponca City','OK');
INSERT INTO `zip3` VALUES ('747','Durant','OK');
INSERT INTO `zip3` VALUES ('748','Shawnee','OK');
INSERT INTO `zip3` VALUES ('749','Poteau','OK');
INSERT INTO `zip3` VALUES ('750','North Texas','TX');
INSERT INTO `zip3` VALUES ('751','Dallas','TX');
INSERT INTO `zip3` VALUES ('752','Dallas','TX');
INSERT INTO `zip3` VALUES ('753','Dallas','TX');
INSERT INTO `zip3` VALUES ('754','Greenville','TX');
INSERT INTO `zip3` VALUES ('755','Texarkana','TX');
INSERT INTO `zip3` VALUES ('756','East Texas','TX');
INSERT INTO `zip3` VALUES ('757','East Texas','TX');
INSERT INTO `zip3` VALUES ('758','Palestine','TX');
INSERT INTO `zip3` VALUES ('759','Lufkin','TX');
INSERT INTO `zip3` VALUES ('760','Ft Worth','TX');
INSERT INTO `zip3` VALUES ('761','Ft Worth','TX');
INSERT INTO `zip3` VALUES ('762','Ft Worth','TX');
INSERT INTO `zip3` VALUES ('763','Wichita Falls','TX');
INSERT INTO `zip3` VALUES ('764','Ft Worth','TX');
INSERT INTO `zip3` VALUES ('765','Waco','TX');
INSERT INTO `zip3` VALUES ('766','Waco','TX');
INSERT INTO `zip3` VALUES ('767','Waco','TX');
INSERT INTO `zip3` VALUES ('768','Abilene','TX');
INSERT INTO `zip3` VALUES ('769','Midland','TX');
INSERT INTO `zip3` VALUES ('770','Houston','TX');
INSERT INTO `zip3` VALUES ('771','Houston','TX');
INSERT INTO `zip3` VALUES ('772','Houston','TX');
INSERT INTO `zip3` VALUES ('773','North Houston','TX');
INSERT INTO `zip3` VALUES ('774','North Houston','TX');
INSERT INTO `zip3` VALUES ('775','North Houston','TX');
INSERT INTO `zip3` VALUES ('776','Beaumont','TX');
INSERT INTO `zip3` VALUES ('777','Beaumont','TX');
INSERT INTO `zip3` VALUES ('778','Bryan','TX');
INSERT INTO `zip3` VALUES ('779','Victoria','TX');
INSERT INTO `zip3` VALUES ('780','San Antonio','TX');
INSERT INTO `zip3` VALUES ('781','San Antonio','TX');
INSERT INTO `zip3` VALUES ('782','San Antonio','TX');
INSERT INTO `zip3` VALUES ('783','Corpus Christi','TX');
INSERT INTO `zip3` VALUES ('784','Corpus Christi','TX');
INSERT INTO `zip3` VALUES ('785','Mcallen','TX');
INSERT INTO `zip3` VALUES ('786','Austin','TX');
INSERT INTO `zip3` VALUES ('787','Austin','TX');
INSERT INTO `zip3` VALUES ('788','San Antonio','TX');
INSERT INTO `zip3` VALUES ('789','Austin','TX');
INSERT INTO `zip3` VALUES ('790','Amarillo','TX');
INSERT INTO `zip3` VALUES ('791','Amarillo','TX');
INSERT INTO `zip3` VALUES ('792','Childress','TX');
INSERT INTO `zip3` VALUES ('793','Lubbock','TX');
INSERT INTO `zip3` VALUES ('794','Lubbock','TX');
INSERT INTO `zip3` VALUES ('795','Abilene','TX');
INSERT INTO `zip3` VALUES ('796','Abilene','TX');
INSERT INTO `zip3` VALUES ('797','Midland','TX');
INSERT INTO `zip3` VALUES ('798','El Paso','TX');
INSERT INTO `zip3` VALUES ('799','El Paso','TX');
INSERT INTO `zip3` VALUES ('800','Denver','CO');
INSERT INTO `zip3` VALUES ('801','Denver','CO');
INSERT INTO `zip3` VALUES ('802','Denver','CO');
INSERT INTO `zip3` VALUES ('803','Boulder','CO');
INSERT INTO `zip3` VALUES ('804','Denver','CO');
INSERT INTO `zip3` VALUES ('805','Longmont','CO');
INSERT INTO `zip3` VALUES ('806','Denver','CO');
INSERT INTO `zip3` VALUES ('807','Denver','CO');
INSERT INTO `zip3` VALUES ('808','Colorado Spgs','CO');
INSERT INTO `zip3` VALUES ('809','Colorado Spgs','CO');
INSERT INTO `zip3` VALUES ('810','Colorado Spgs','CO');
INSERT INTO `zip3` VALUES ('811','Alamosa','CO');
INSERT INTO `zip3` VALUES ('812','Salida','CO');
INSERT INTO `zip3` VALUES ('813','Durango','CO');
INSERT INTO `zip3` VALUES ('814','Grand Junction','CO');
INSERT INTO `zip3` VALUES ('815','Grand Junction','CO');
INSERT INTO `zip3` VALUES ('816','Glenwood Springs','CO');
INSERT INTO `zip3` VALUES ('820','Cheyenne','WY');
INSERT INTO `zip3` VALUES ('821','Yellowstone Nl Pk','WY');
INSERT INTO `zip3` VALUES ('822','Wheatland','WY');
INSERT INTO `zip3` VALUES ('823','Rawlins','WY');
INSERT INTO `zip3` VALUES ('824','Worland','WY');
INSERT INTO `zip3` VALUES ('825','Riverton','WY');
INSERT INTO `zip3` VALUES ('826','Casper','WY');
INSERT INTO `zip3` VALUES ('827','Gillette','WY');
INSERT INTO `zip3` VALUES ('828','Sheridan','WY');
INSERT INTO `zip3` VALUES ('829','Rock Springs','WY');
INSERT INTO `zip3` VALUES ('830','Rock Springs','WY');
INSERT INTO `zip3` VALUES ('831','Rock Springs','WY');
INSERT INTO `zip3` VALUES ('832','Pocatello','ID');
INSERT INTO `zip3` VALUES ('833','Twin Falls','ID');
INSERT INTO `zip3` VALUES ('834','Pocatello','ID');
INSERT INTO `zip3` VALUES ('835','Lewiston','ID');
INSERT INTO `zip3` VALUES ('836','Boise','ID');
INSERT INTO `zip3` VALUES ('837','Boise','ID');
INSERT INTO `zip3` VALUES ('838','Spokane','WA');
INSERT INTO `zip3` VALUES ('840','Salt Lake Cty','UT');
INSERT INTO `zip3` VALUES ('841','Salt Lake Cty','UT');
INSERT INTO `zip3` VALUES ('842','Salt Lake Cty','UT');
INSERT INTO `zip3` VALUES ('843','Salt Lake Cty','UT');
INSERT INTO `zip3` VALUES ('844','Ogden','UT');
INSERT INTO `zip3` VALUES ('845','Provo','UT');
INSERT INTO `zip3` VALUES ('846','Provo','UT');
INSERT INTO `zip3` VALUES ('847','Provo','UT');
INSERT INTO `zip3` VALUES ('850','Phoenix','AZ');
INSERT INTO `zip3` VALUES ('852','Phoenix','AZ');
INSERT INTO `zip3` VALUES ('853','Phoenix','AZ');
INSERT INTO `zip3` VALUES ('855','Globe','AZ');
INSERT INTO `zip3` VALUES ('856','Tucson','AZ');
INSERT INTO `zip3` VALUES ('857','Tucson','AZ');
INSERT INTO `zip3` VALUES ('859','Show Low','AZ');
INSERT INTO `zip3` VALUES ('860','Flagstaff','AZ');
INSERT INTO `zip3` VALUES ('863','Prescott','AZ');
INSERT INTO `zip3` VALUES ('864','Kingman','AZ');
INSERT INTO `zip3` VALUES ('865','Gallup','NM');
INSERT INTO `zip3` VALUES ('870','Albuquerque','NM');
INSERT INTO `zip3` VALUES ('871','Albuquerque','NM');
INSERT INTO `zip3` VALUES ('872','Albuquerque','NM');
INSERT INTO `zip3` VALUES ('873','Gallup','NM');
INSERT INTO `zip3` VALUES ('874','Farmington','NM');
INSERT INTO `zip3` VALUES ('875','Albuquerque','NM');
INSERT INTO `zip3` VALUES ('877','Las Vegas','NM');
INSERT INTO `zip3` VALUES ('878','Socorro','NM');
INSERT INTO `zip3` VALUES ('879','Truth Or Cons','NM');
INSERT INTO `zip3` VALUES ('880','Las Cruces','NM');
INSERT INTO `zip3` VALUES ('881','Clovis','NM');
INSERT INTO `zip3` VALUES ('882','Roswell','NM');
INSERT INTO `zip3` VALUES ('883','Alamogordo','NM');
INSERT INTO `zip3` VALUES ('884','Tucumcari','NM');
INSERT INTO `zip3` VALUES ('885','El Paso','TX');
INSERT INTO `zip3` VALUES ('889','Las Vegas','NV');
INSERT INTO `zip3` VALUES ('890','Las Vegas','NV');
INSERT INTO `zip3` VALUES ('891','Las Vegas','NV');
INSERT INTO `zip3` VALUES ('893','Ely','NV');
INSERT INTO `zip3` VALUES ('894','Reno','NV');
INSERT INTO `zip3` VALUES ('895','Reno','NV');
INSERT INTO `zip3` VALUES ('897','Carson City','NV');
INSERT INTO `zip3` VALUES ('898','Elko','NV');
INSERT INTO `zip3` VALUES ('900','Los Angeles','CA');
INSERT INTO `zip3` VALUES ('901','Los Angeles','CA');
INSERT INTO `zip3` VALUES ('902','Inglewood','CA');
INSERT INTO `zip3` VALUES ('903','Inglewood','CA');
INSERT INTO `zip3` VALUES ('904','Santa Monica','CA');
INSERT INTO `zip3` VALUES ('905','Torrance','CA');
INSERT INTO `zip3` VALUES ('906','Long Beach','CA');
INSERT INTO `zip3` VALUES ('907','Long Beach','CA');
INSERT INTO `zip3` VALUES ('908','Long Beach','CA');
INSERT INTO `zip3` VALUES ('910','Pasadena','CA');
INSERT INTO `zip3` VALUES ('911','Pasadena','CA');
INSERT INTO `zip3` VALUES ('912','Glendale','CA');
INSERT INTO `zip3` VALUES ('913','Van Nuys','CA');
INSERT INTO `zip3` VALUES ('914','Van Nuys','CA');
INSERT INTO `zip3` VALUES ('915','Burbank','CA');
INSERT INTO `zip3` VALUES ('916','North Hollywood','CA');
INSERT INTO `zip3` VALUES ('917','Industry','CA');
INSERT INTO `zip3` VALUES ('918','Industry','CA');
INSERT INTO `zip3` VALUES ('919','San Diego','CA');
INSERT INTO `zip3` VALUES ('920','San Diego','CA');
INSERT INTO `zip3` VALUES ('921','San Diego','CA');
INSERT INTO `zip3` VALUES ('922','Sn Bernardino','CA');
INSERT INTO `zip3` VALUES ('923','Sn Bernardino','CA');
INSERT INTO `zip3` VALUES ('924','Sn Bernardino','CA');
INSERT INTO `zip3` VALUES ('925','Sn Bernardino','CA');
INSERT INTO `zip3` VALUES ('926','Santa Ana','CA');
INSERT INTO `zip3` VALUES ('927','Santa Ana','CA');
INSERT INTO `zip3` VALUES ('928','Anaheim','CA');
INSERT INTO `zip3` VALUES ('930','Oxnard','CA');
INSERT INTO `zip3` VALUES ('931','Santa Barbara','CA');
INSERT INTO `zip3` VALUES ('932','Bakersfield','CA');
INSERT INTO `zip3` VALUES ('933','Bakersfield','CA');
INSERT INTO `zip3` VALUES ('934','Santa Barbara','CA');
INSERT INTO `zip3` VALUES ('935','Mojave','CA');
INSERT INTO `zip3` VALUES ('936','Fresno','CA');
INSERT INTO `zip3` VALUES ('937','Fresno','CA');
INSERT INTO `zip3` VALUES ('938','Fresno','CA');
INSERT INTO `zip3` VALUES ('939','Salinas','CA');
INSERT INTO `zip3` VALUES ('940','San Francisco','CA');
INSERT INTO `zip3` VALUES ('941','San Francisco','CA');
INSERT INTO `zip3` VALUES ('942','Sacramento','CA');
INSERT INTO `zip3` VALUES ('943','Palo Alto','CA');
INSERT INTO `zip3` VALUES ('944','San Mateo','CA');
INSERT INTO `zip3` VALUES ('945','Oakland','CA');
INSERT INTO `zip3` VALUES ('946','Oakland','CA');
INSERT INTO `zip3` VALUES ('947','Berkeley','CA');
INSERT INTO `zip3` VALUES ('948','Richmond','CA');
INSERT INTO `zip3` VALUES ('949','North Bay','CA');
INSERT INTO `zip3` VALUES ('950','San Jose','CA');
INSERT INTO `zip3` VALUES ('951','San Jose','CA');
INSERT INTO `zip3` VALUES ('952','Stockton','CA');
INSERT INTO `zip3` VALUES ('953','Stockton','CA');
INSERT INTO `zip3` VALUES ('954','North Bay','CA');
INSERT INTO `zip3` VALUES ('955','Eureka','CA');
INSERT INTO `zip3` VALUES ('956','Sacramento','CA');
INSERT INTO `zip3` VALUES ('957','Sacramento','CA');
INSERT INTO `zip3` VALUES ('958','Sacramento','CA');
INSERT INTO `zip3` VALUES ('959','Marysville','CA');
INSERT INTO `zip3` VALUES ('960','Redding','CA');
INSERT INTO `zip3` VALUES ('961','Reno','NV');
INSERT INTO `zip3` VALUES ('962','Apo/Fpo','AP');
INSERT INTO `zip3` VALUES ('963','Apo/Fpo','AP');
INSERT INTO `zip3` VALUES ('964','Apo/Fpo','AP');
INSERT INTO `zip3` VALUES ('965','Apo/Fpo','AP');
INSERT INTO `zip3` VALUES ('966','Fpo','AP');
INSERT INTO `zip3` VALUES ('967','Honolulu','HI');
INSERT INTO `zip3` VALUES ('968','Honolulu','HI');
INSERT INTO `zip3` VALUES ('969','Barrigada','GU');
INSERT INTO `zip3` VALUES ('970','Portland','OR');
INSERT INTO `zip3` VALUES ('971','Portland','OR');
INSERT INTO `zip3` VALUES ('972','Portland','OR');
INSERT INTO `zip3` VALUES ('973','Salem','OR');
INSERT INTO `zip3` VALUES ('974','Eugene','OR');
INSERT INTO `zip3` VALUES ('975','Medford','OR');
INSERT INTO `zip3` VALUES ('976','Klamath Falls','OR');
INSERT INTO `zip3` VALUES ('977','Bend','OR');
INSERT INTO `zip3` VALUES ('978','Pendleton','OR');
INSERT INTO `zip3` VALUES ('979','Boise','ID');
INSERT INTO `zip3` VALUES ('980','Seattle','WA');
INSERT INTO `zip3` VALUES ('981','Seattle','WA');
INSERT INTO `zip3` VALUES ('982','Everett','WA');
INSERT INTO `zip3` VALUES ('983','Tacoma','WA');
INSERT INTO `zip3` VALUES ('984','Tacoma','WA');
INSERT INTO `zip3` VALUES ('985','Olympia','WA');
INSERT INTO `zip3` VALUES ('986','Portland','OR');
INSERT INTO `zip3` VALUES ('988','Wenatchee','WA');
INSERT INTO `zip3` VALUES ('989','Yakima','WA');
INSERT INTO `zip3` VALUES ('990','Spokane','WA');
INSERT INTO `zip3` VALUES ('991','Spokane','WA');
INSERT INTO `zip3` VALUES ('992','Spokane','WA');
INSERT INTO `zip3` VALUES ('993','Pasco','WA');
INSERT INTO `zip3` VALUES ('994','Lewiston','ID');
INSERT INTO `zip3` VALUES ('995','Anchorage','AK');
INSERT INTO `zip3` VALUES ('996','Anchorage','AK');
INSERT INTO `zip3` VALUES ('997','Fairbanks','AK');
INSERT INTO `zip3` VALUES ('998','Juneau','AK');
INSERT INTO `zip3` VALUES ('999','Ketchikan','AK');
/*!40000 ALTER TABLE `zip3` ENABLE KEYS */;
UNLOCK TABLES;

LOCK TABLES `zips` WRITE;
/*!40000 ALTER TABLE `zips` DISABLE KEYS */;
INSERT INTO `zips` VALUES (100,'STANDARD',0x00,'Somewhere',NULL,'Sometown','AL','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (101,'STANDARD',0x00,'Somewhere',NULL,'Sometown','AK','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (102,'STANDARD',0x00,'Somewhere',NULL,'Sometown','AZ','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (103,'STANDARD',0x00,'Somewhere',NULL,'Sometown','AR','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (104,'STANDARD',0x00,'Somewhere',NULL,'Sometown','CA','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (105,'STANDARD',0x00,'Somewhere',NULL,'Sometown','CO','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (106,'STANDARD',0x00,'Somewhere',NULL,'Sometown','CT','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (107,'STANDARD',0x00,'Somewhere',NULL,'Sometown','DE','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (108,'STANDARD',0x00,'Somewhere',NULL,'Sometown','FL','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (109,'STANDARD',0x00,'Somewhere',NULL,'Sometown','GA','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (110,'STANDARD',0x00,'Somewhere',NULL,'Sometown','HI','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (111,'STANDARD',0x00,'Somewhere',NULL,'Sometown','ID','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (112,'STANDARD',0x00,'Somewhere',NULL,'Sometown','IL','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (113,'STANDARD',0x00,'Somewhere',NULL,'Sometown','IN','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (114,'STANDARD',0x00,'Somewhere',NULL,'Sometown','IA','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (115,'STANDARD',0x00,'Somewhere',NULL,'Sometown','KS','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (116,'STANDARD',0x00,'Somewhere',NULL,'Sometown','KY','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (117,'STANDARD',0x00,'Somewhere',NULL,'Sometown','LA','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (118,'STANDARD',0x00,'Somewhere',NULL,'Sometown','ME','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (119,'STANDARD',0x00,'Somewhere',NULL,'Sometown','MD','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (120,'STANDARD',0x00,'Somewhere',NULL,'Sometown','MA','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (121,'STANDARD',0x00,'Somewhere',NULL,'Sometown','MI','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (122,'STANDARD',0x00,'Somewhere',NULL,'Sometown','MN','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (123,'STANDARD',0x00,'Somewhere',NULL,'Sometown','MS','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (124,'STANDARD',0x00,'Somewhere',NULL,'Sometown','MO','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (125,'STANDARD',0x00,'Somewhere',NULL,'Sometown','MT','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (126,'STANDARD',0x00,'Somewhere',NULL,'Sometown','NE','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (127,'STANDARD',0x00,'Somewhere',NULL,'Sometown','NV','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (128,'STANDARD',0x00,'Somewhere',NULL,'Sometown','NH','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (129,'STANDARD',0x00,'Somewhere',NULL,'Sometown','NJ','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (130,'STANDARD',0x00,'Somewhere',NULL,'Sometown','NM','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (131,'STANDARD',0x00,'Somewhere',NULL,'Sometown','NY','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (132,'STANDARD',0x00,'Somewhere',NULL,'Sometown','NC','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (133,'STANDARD',0x00,'Somewhere',NULL,'Sometown','ND','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (134,'STANDARD',0x00,'Somewhere',NULL,'Sometown','OH','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (135,'STANDARD',0x00,'Somewhere',NULL,'Sometown','OK','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (136,'STANDARD',0x00,'Somewhere',NULL,'Sometown','OR','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (137,'STANDARD',0x00,'Somewhere',NULL,'Sometown','PA','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (138,'STANDARD',0x00,'Somewhere',NULL,'Sometown','RI','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (139,'STANDARD',0x00,'Somewhere',NULL,'Sometown','SC','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (140,'STANDARD',0x00,'Somewhere',NULL,'Sometown','SD','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (141,'STANDARD',0x00,'Somewhere',NULL,'Sometown','TN','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (142,'STANDARD',0x00,'Somewhere',NULL,'Sometown','TX','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (143,'STANDARD',0x00,'Somewhere',NULL,'Sometown','UT','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (144,'STANDARD',0x00,'Somewhere',NULL,'Sometown','VT','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (145,'STANDARD',0x00,'Somewhere',NULL,'Sometown','VA','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (146,'STANDARD',0x00,'Somewhere',NULL,'Sometown','WA','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (147,'STANDARD',0x00,'Somewhere',NULL,'Sometown','WV','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (148,'STANDARD',0x00,'Somewhere',NULL,'Sometown','WI','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (149,'STANDARD',0x00,'Somewhere',NULL,'Sometown','WY','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (150,'STANDARD',0x00,'Somewhere',NULL,'Sometown','DC','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (152,'STANDARD',0x00,'Somewhere',NULL,'Sometown','AS','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (153,'STANDARD',0x00,'Somewhere',NULL,'Sometown','GU','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (155,'STANDARD',0x00,'Somewhere',NULL,'Sometown','MP','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (156,'STANDARD',0x00,'Somewhere',NULL,'Sometown','PR','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (157,'STANDARD',0x00,'Somewhere',NULL,'Sometown','VI','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (158,'STANDARD',0x00,'Somewhere',NULL,'Sometown','UM','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (159,'STANDARD',0x00,'Somewhere',NULL,'Sometown','AE','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (160,'STANDARD',0x00,'Somewhere',NULL,'Sometown','AA','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (161,'STANDARD',0x00,'Somewhere',NULL,'Sometown','AP','Other County','USA','401','NA','US',35.00,75.00,2500);
INSERT INTO `zips` VALUES (1001,'STANDARD',0x00,'Agawam',NULL,NULL,'MA','Hampden County','America/New_York','413','NA','US',42.06,-72.61,15220);
INSERT INTO `zips` VALUES (1002,'STANDARD',0x00,'Amherst','Cushman, Pelham','South Amherst','MA','Hampshire County','America/New_York','413','NA','US',42.37,-72.52,16570);
INSERT INTO `zips` VALUES (1003,'PO BOX',0x00,'Amherst',NULL,NULL,'MA','Hampshire County','America/New_York','413','NA','US',42.39,-72.52,184);
INSERT INTO `zips` VALUES (1004,'PO BOX',0x00,'Amherst',NULL,NULL,'MA','Hampshire County','America/New_York','413','NA','US',42.37,-72.52,794);
INSERT INTO `zips` VALUES (1005,'STANDARD',0x00,'Barre',NULL,NULL,'MA','Worcester County','America/New_York','978','NA','US',42.42,-72.10,4270);
INSERT INTO `zips` VALUES (1007,'STANDARD',0x00,'Belchertown',NULL,NULL,'MA','Hampshire County','America/New_York','413','NA','US',42.27,-72.40,14180);
INSERT INTO `zips` VALUES (1008,'STANDARD',0x00,'Blandford',NULL,NULL,'MA','Hampden County','America/New_York','413','NA','US',42.18,-72.93,1140);
INSERT INTO `zips` VALUES (1009,'PO BOX',0x00,'Bondsville',NULL,NULL,'MA','Hampden County','America/New_York','413','NA','US',42.20,-72.34,1238);
INSERT INTO `zips` VALUES (1010,'STANDARD',0x00,'Brimfield',NULL,'East Brimfield','MA','Hampden County','America/New_York','413','NA','US',42.11,-72.20,3420);
INSERT INTO `zips` VALUES (1011,'STANDARD',0x00,'Chester',NULL,NULL,'MA','Hampden County','America/New_York','413','NA','US',42.28,-72.98,1090);
INSERT INTO `zips` VALUES (1012,'STANDARD',0x00,'Chesterfield',NULL,NULL,'MA','Hampshire County','America/New_York','413','NA','US',42.40,-72.85,610);
INSERT INTO `zips` VALUES (1013,'STANDARD',0x00,'Chicopee','Willimansett',NULL,'MA','Hampden County','America/New_York','413','NA','US',42.15,-72.60,18880);
INSERT INTO `zips` VALUES (1014,'PO BOX',0x00,'Chicopee',NULL,NULL,'MA','Hampden County','America/New_York','413','NA','US',42.17,-72.57,354);
INSERT INTO `zips` VALUES (1020,'STANDARD',0x00,'Chicopee',NULL,NULL,'MA','Hampden County','America/New_York','413','NA','US',42.17,-72.57,25790);
INSERT INTO `zips` VALUES (1021,'PO BOX',0x00,'Chicopee',NULL,NULL,'MA','Hampden County','America/New_York','413','NA','US',42.17,-72.57,566);
INSERT INTO `zips` VALUES (1022,'STANDARD',0x00,'Chicopee','Westover Afb',NULL,'MA','Hampden County','America/New_York','413','NA','US',42.20,-72.54,1780);
INSERT INTO `zips` VALUES (1026,'STANDARD',0x00,'Cummington',NULL,'West Cummington','MA','Hampshire County','America/New_York','413','NA','US',42.46,-72.90,890);
INSERT INTO `zips` VALUES (1027,'STANDARD',0x00,'Easthampton','E Hampton, Mount Tom, Westhampton','Loudville','MA','Hampshire County','America/New_York','413','NA','US',42.26,-72.68,16320);
INSERT INTO `zips` VALUES (1028,'STANDARD',0x00,'East Longmeadow','E Longmeadow',NULL,'MA','Hampden County','America/New_York','413','NA','US',42.06,-72.51,15330);
INSERT INTO `zips` VALUES (1029,'PO BOX',0x00,'East Otis',NULL,'Big Pond, E Otis','MA','Berkshire County','America/New_York','413','NA','US',42.17,-73.03,557);
INSERT INTO `zips` VALUES (1030,'STANDARD',0x00,'Feeding Hills',NULL,NULL,'MA','Hampden County','America/New_York','413','NA','US',42.06,-72.67,11010);
INSERT INTO `zips` VALUES (1031,'STANDARD',0x00,'Gilbertville',NULL,'Old Furnace','MA','Worcester County','America/New_York','413','NA','US',42.33,-72.20,1000);
INSERT INTO `zips` VALUES (1032,'STANDARD',0x00,'Goshen',NULL,'Lithia','MA','Hampshire County','America/New_York','413','NA','US',42.46,-72.81,470);
INSERT INTO `zips` VALUES (1033,'STANDARD',0x00,'Granby',NULL,NULL,'MA','Hampshire County','America/New_York','413','NA','US',42.26,-72.52,5890);
INSERT INTO `zips` VALUES (1034,'STANDARD',0x00,'Granville','Tolland','Granville Center, West Granville','MA','Hampden County','America/New_York','413','NA','US',42.06,-72.86,1860);
INSERT INTO `zips` VALUES (1035,'STANDARD',0x00,'Hadley',NULL,'North Hadley','MA','Hampshire County','America/New_York','413','NA','US',42.35,-72.58,4620);
INSERT INTO `zips` VALUES (1036,'STANDARD',0x00,'Hampden',NULL,'Hampton','MA','Hampden County','America/New_York','413','NA','US',42.06,-72.41,4880);
INSERT INTO `zips` VALUES (1037,'PO BOX',0x00,'Hardwick',NULL,NULL,'MA','Worcester County','America/New_York',NULL,'NA','US',42.35,-72.20,779);
INSERT INTO `zips` VALUES (1038,'STANDARD',0x00,'Hatfield',NULL,'West Hatfield','MA','Hampshire County','America/New_York','413','NA','US',42.37,-72.60,2280);
INSERT INTO `zips` VALUES (1039,'STANDARD',0x00,'Haydenville','West Whately',NULL,'MA','Hampshire County','America/New_York',NULL,'NA','US',42.39,-72.70,1240);
INSERT INTO `zips` VALUES (1040,'STANDARD',0x00,'Holyoke',NULL,'Halyoke','MA','Hampden County','America/New_York','413','NA','US',42.21,-72.64,29750);
INSERT INTO `zips` VALUES (1041,'PO BOX',0x00,'Holyoke',NULL,'Halyoke','MA','Hampden County','America/New_York','413','NA','US',42.21,-72.64,1408);
INSERT INTO `zips` VALUES (1050,'STANDARD',0x00,'Huntington','Montgomery','Crescent Mills, Hntgtn, Knightville, North Chester, South Worthington','MA','Hampshire County','America/New_York','413','NA','US',42.23,-72.88,2270);
INSERT INTO `zips` VALUES (1053,'STANDARD',0x00,'Leeds',NULL,NULL,'MA','Hampshire County','America/New_York','413','NA','US',42.35,-72.71,1520);
INSERT INTO `zips` VALUES (1054,'STANDARD',0x00,'Leverett',NULL,'East Leverett, North Leverett','MA','Franklin County','America/New_York',NULL,'NA','US',42.45,-72.50,1750);
INSERT INTO `zips` VALUES (1056,'STANDARD',0x00,'Ludlow',NULL,NULL,'MA','Hampden County','America/New_York','413','NA','US',42.16,-72.48,18130);
INSERT INTO `zips` VALUES (1057,'STANDARD',0x00,'Monson',NULL,NULL,'MA','Hampden County','America/New_York','413','NA','US',42.09,-72.31,7600);
INSERT INTO `zips` VALUES (1059,'PO BOX',0x00,'North Amherst','Amherst',NULL,'MA','Hampshire County','America/New_York','413','NA','US',42.40,-72.52,70);
INSERT INTO `zips` VALUES (1060,'STANDARD',0x00,'Northampton',NULL,'North Hampton','MA','Hampshire County','America/New_York','413','NA','US',42.32,-72.63,11260);
INSERT INTO `zips` VALUES (1061,'PO BOX',0x00,'Northampton',NULL,'North Hampton','MA','Hampshire County','America/New_York','413','NA','US',42.32,-72.67,469);
INSERT INTO `zips` VALUES (1062,'STANDARD',0x00,'Florence','Bay State Village, Bay State Vlg, Northampton','North Hampton','MA','Hampshire County','America/New_York','413','NA','US',42.32,-72.67,10090);
INSERT INTO `zips` VALUES (1063,'UNIQUE',0x00,'Northampton',NULL,'North Hampton, Smith College','MA','Hampshire County','America/New_York','413','NA','US',42.32,-72.64,177);
INSERT INTO `zips` VALUES (1066,'PO BOX',0x00,'North Hatfield','N Hatfield','No Hatfield','MA','Hampshire County','America/New_York','413','NA','US',42.41,-72.66,402);
INSERT INTO `zips` VALUES (1068,'STANDARD',0x00,'Oakham',NULL,NULL,'MA','Worcester County','America/New_York','508,774','NA','US',42.35,-72.05,1790);
INSERT INTO `zips` VALUES (1069,'STANDARD',0x00,'Palmer',NULL,NULL,'MA','Hampden County','America/New_York','413','NA','US',42.16,-72.32,7190);
INSERT INTO `zips` VALUES (1070,'STANDARD',0x00,'Plainfield',NULL,NULL,'MA','Hampshire County','America/New_York','413','NA','US',42.51,-72.91,540);
INSERT INTO `zips` VALUES (1071,'STANDARD',0x00,'Russell',NULL,NULL,'MA','Hampden County','America/New_York','413','NA','US',42.18,-72.85,1390);
INSERT INTO `zips` VALUES (1072,'STANDARD',0x00,'Shutesbury',NULL,NULL,'MA','Franklin County','America/New_York',NULL,'NA','US',42.45,-72.40,1350);
INSERT INTO `zips` VALUES (1073,'STANDARD',0x00,'Southampton',NULL,NULL,'MA','Hampshire County','America/New_York','413','NA','US',42.23,-72.73,5980);
INSERT INTO `zips` VALUES (1074,'PO BOX',0x00,'South Barre',NULL,NULL,'MA','Worcester County','America/New_York','351','NA','US',42.39,-72.09,655);
INSERT INTO `zips` VALUES (1075,'STANDARD',0x00,'South Hadley',NULL,'S Hadley, So Hadley, South Hadley Falls','MA','Hampshire County','America/New_York','413','NA','US',42.26,-72.56,14590);
INSERT INTO `zips` VALUES (1077,'STANDARD',0x00,'Southwick',NULL,NULL,'MA','Hampden County','America/New_York','413','NA','US',42.05,-72.76,8820);
INSERT INTO `zips` VALUES (1079,'PO BOX',0x00,'Thorndike',NULL,NULL,'MA','Hampden County','America/New_York','413','NA','US',42.20,-72.33,992);
INSERT INTO `zips` VALUES (1080,'STANDARD',0x00,'Three Rivers',NULL,NULL,'MA','Hampden County','America/New_York','413','NA','US',42.18,-72.37,2000);
INSERT INTO `zips` VALUES (1081,'STANDARD',0x00,'Wales',NULL,NULL,'MA','Hampden County','America/New_York','413','NA','US',42.06,-72.21,1490);
INSERT INTO `zips` VALUES (1082,'STANDARD',0x00,'Ware','Hardwick',NULL,'MA','Hampshire County','America/New_York','413','NA','US',42.25,-72.24,8840);
INSERT INTO `zips` VALUES (1083,'PO BOX',0x00,'Warren',NULL,NULL,'MA','Worcester County','America/New_York',NULL,'NA','US',42.21,-72.19,3037);
INSERT INTO `zips` VALUES (1084,'STANDARD',0x00,'West Chesterfield','W Chesterfld',NULL,'MA','Hampshire County','America/New_York','413','NA','US',42.39,-72.88,137);
INSERT INTO `zips` VALUES (1085,'STANDARD',0x00,'Westfield','Montgomery',NULL,'MA','Hampden County','America/New_York','413','NA','US',42.13,-72.75,34780);
INSERT INTO `zips` VALUES (1086,'PO BOX',0x00,'Westfield',NULL,NULL,'MA','Hampden County','America/New_York','413','NA','US',42.13,-72.79,886);
INSERT INTO `zips` VALUES (1088,'STANDARD',0x00,'West Hatfield','W Hatfield',NULL,'MA','Hampshire County','America/New_York','413','NA','US',42.39,-72.64,490);
INSERT INTO `zips` VALUES (1089,'STANDARD',0x00,'West Springfield','W Springfield','West Springfld','MA','Hampden County','America/New_York','413','NA','US',42.12,-72.65,25520);
INSERT INTO `zips` VALUES (1090,'PO BOX',0x00,'West Springfield','W Springfield',NULL,'MA','Hampden County','America/New_York','413','NA','US',42.12,-72.65,510);
INSERT INTO `zips` VALUES (1092,'PO BOX',0x00,'West Warren',NULL,'W Warren','MA','Worcester County','America/New_York',NULL,'NA','US',42.19,-72.24,1332);
INSERT INTO `zips` VALUES (1093,'PO BOX',0x00,'Whately',NULL,NULL,'MA','Franklin County','America/New_York',NULL,'NA','US',42.44,-72.65,516);
INSERT INTO `zips` VALUES (1094,'PO BOX',0x00,'Wheelwright',NULL,NULL,'MA','Worcester County','America/New_York',NULL,'NA','US',42.35,-72.14,340);
INSERT INTO `zips` VALUES (1095,'STANDARD',0x00,'Wilbraham',NULL,NULL,'MA','Hampden County','America/New_York','413','NA','US',42.13,-72.43,13840);
INSERT INTO `zips` VALUES (1096,'STANDARD',0x00,'Williamsburg',NULL,'S Chesterfield, South Chesterfield','MA','Hampshire County','America/New_York','413','NA','US',42.40,-72.76,2310);
INSERT INTO `zips` VALUES (1097,'PO BOX',0x00,'Woronoco',NULL,NULL,'MA','Hampden County','America/New_York','413','NA','US',42.18,-72.83,88);
INSERT INTO `zips` VALUES (1098,'STANDARD',0x00,'Worthington',NULL,NULL,'MA','Hampshire County','America/New_York','413','NA','US',42.41,-72.93,1060);
INSERT INTO `zips` VALUES (1101,'PO BOX',0x00,'Springfield',NULL,'Spfld','MA','Hampden County','America/New_York','413','NA','US',42.11,-72.53,2038);
INSERT INTO `zips` VALUES (1102,'PO BOX',0x00,'Springfield',NULL,'Spfld','MA','Hampden County','America/New_York','413','NA','US',42.11,-72.53,0);
INSERT INTO `zips` VALUES (1103,'STANDARD',0x00,'Springfield',NULL,'Spfld','MA','Hampden County','America/New_York','413','NA','US',42.10,-72.59,1580);
INSERT INTO `zips` VALUES (1104,'STANDARD',0x00,'Springfield',NULL,'Spfld','MA','Hampden County','America/New_York','413','NA','US',42.13,-72.57,19290);
INSERT INTO `zips` VALUES (1105,'STANDARD',0x00,'Springfield',NULL,'Spfld','MA','Hampden County','America/New_York','413','NA','US',42.10,-72.58,8040);
INSERT INTO `zips` VALUES (1106,'STANDARD',0x00,'Longmeadow','Spfld, Springfield',NULL,'MA','Hampden County','America/New_York','413','NA','US',42.04,-72.57,15400);
INSERT INTO `zips` VALUES (1107,'STANDARD',0x00,'Springfield',NULL,'Brightwood, Spfld','MA','Hampden County','America/New_York','413','NA','US',42.12,-72.61,8420);
INSERT INTO `zips` VALUES (1108,'STANDARD',0x00,'Springfield',NULL,'Spfld','MA','Hampden County','America/New_York','413','NA','US',42.08,-72.56,21860);
INSERT INTO `zips` VALUES (1109,'STANDARD',0x00,'Springfield',NULL,'Spfld','MA','Hampden County','America/New_York','413','NA','US',42.11,-72.53,21920);
INSERT INTO `zips` VALUES (1111,'UNIQUE',0x00,'Springfield',NULL,'Mass Mutual Life Ins Co','MA','Hampden County','America/New_York','413','NA','US',42.11,-72.53,0);
INSERT INTO `zips` VALUES (1115,'PO BOX',0x00,'Springfield',NULL,'Bay State W Tower','MA','Hampden County','America/New_York','413','NA','US',42.11,-72.53,25);
INSERT INTO `zips` VALUES (1116,'PO BOX',0x00,'Longmeadow','E Longmeadow, East Longmeadow',NULL,'MA','Hampden County','America/New_York','413','NA','US',42.04,-72.57,82);
INSERT INTO `zips` VALUES (1118,'STANDARD',0x00,'Springfield',NULL,'Spfld','MA','Hampden County','America/New_York','413','NA','US',42.09,-72.53,12820);
INSERT INTO `zips` VALUES (1119,'STANDARD',0x00,'Springfield',NULL,'Spfld','MA','Hampden County','America/New_York','413','NA','US',42.12,-72.51,11040);
INSERT INTO `zips` VALUES (1128,'STANDARD',0x00,'Springfield',NULL,'Spfld','MA','Hampden County','America/New_York','413','NA','US',42.09,-72.49,2610);
INSERT INTO `zips` VALUES (1129,'STANDARD',0x00,'Springfield',NULL,'Spfld','MA','Hampden County','America/New_York','413','NA','US',42.12,-72.49,6540);
INSERT INTO `zips` VALUES (1133,'UNIQUE',0x01,'Springfield',NULL,'Monarch Life Ins Co','MA','Hampden County','America/New_York','413','NA','US',42.10,-72.59,0);
INSERT INTO `zips` VALUES (1138,'PO BOX',0x00,'Springfield',NULL,'Spfld','MA','Hampden County','America/New_York','413','NA','US',42.11,-72.53,688);
INSERT INTO `zips` VALUES (1139,'PO BOX',0x00,'Springfield',NULL,'Spfld','MA','Hampden County','America/New_York','413','NA','US',42.11,-72.53,467);
INSERT INTO `zips` VALUES (1144,'STANDARD',0x00,'Springfield',NULL,NULL,'MA','Hampden County','America/New_York','413','NA','US',42.11,-72.53,0);
INSERT INTO `zips` VALUES (1151,'STANDARD',0x00,'Indian Orchard','Indian Orch, Springfield','Spfld','MA','Hampden County','America/New_York','413','NA','US',42.15,-72.51,7630);
INSERT INTO `zips` VALUES (1152,'STANDARD',0x00,'Springfield',NULL,'General Mail Facility-bmc','MA','Hampden County','America/New_York','413','NA','US',42.11,-72.53,0);
INSERT INTO `zips` VALUES (1195,'STANDARD',0x01,'Springfield','Springfield Bmc',NULL,'MA','Hampden County','America/New_York','413','NA','US',42.10,-72.58,0);
INSERT INTO `zips` VALUES (1199,'UNIQUE',0x00,'Springfield',NULL,'Baystate Medical, Spfld','MA','Hampden County','America/New_York','413','NA','US',42.12,-72.60,0);
INSERT INTO `zips` VALUES (1201,'STANDARD',0x00,'Pittsfield',NULL,'Allendale','MA','Berkshire County','America/New_York','413','NA','US',42.45,-73.26,38200);
INSERT INTO `zips` VALUES (1202,'PO BOX',0x00,'Pittsfield',NULL,NULL,'MA','Berkshire County','America/New_York','413','NA','US',42.45,-73.26,1387);
INSERT INTO `zips` VALUES (1203,'PO BOX',0x00,'Pittsfield',NULL,NULL,'MA','Berkshire County','America/New_York','413','NA','US',42.45,-73.26,0);
INSERT INTO `zips` VALUES (1220,'STANDARD',0x00,'Adams',NULL,NULL,'MA','Berkshire County','America/New_York','413','NA','US',42.62,-73.11,7260);
INSERT INTO `zips` VALUES (1222,'STANDARD',0x00,'Ashley Falls',NULL,NULL,'MA','Berkshire County','America/New_York','413','NA','US',42.05,-73.33,730);
INSERT INTO `zips` VALUES (1223,'STANDARD',0x00,'Becket','Washington','Becket Corners, Sherwood Forest','MA','Berkshire County','America/New_York','413','NA','US',42.33,-73.07,1930);
INSERT INTO `zips` VALUES (1224,'STANDARD',0x00,'Berkshire','Lanesboro',NULL,'MA','Berkshire County','America/New_York','413','NA','US',42.51,-73.20,160);
INSERT INTO `zips` VALUES (1225,'STANDARD',0x00,'Cheshire',NULL,NULL,'MA','Berkshire County','America/New_York','413','NA','US',42.55,-73.15,2970);
INSERT INTO `zips` VALUES (1226,'STANDARD',0x00,'Dalton',NULL,NULL,'MA','Berkshire County','America/New_York','413','NA','US',42.47,-73.16,5910);
INSERT INTO `zips` VALUES (1227,'PO BOX',0x00,'Dalton',NULL,NULL,'MA','Berkshire County','America/New_York','413','NA','US',42.47,-73.16,262);
INSERT INTO `zips` VALUES (1229,'PO BOX',0x00,'Glendale',NULL,NULL,'MA','Berkshire County','America/New_York','413','NA','US',42.28,-73.34,163);
INSERT INTO `zips` VALUES (1230,'STANDARD',0x00,'Great Barrington','Egremont, Gt Barrington, N Egremont, New Marlboro, New Marlborou, New Marlborough, North Egremont, Simons Rock','Alford, Berkshire Heights, Hartsville, Risingdale, Van Deusenville','MA','Berkshire County','America/New_York','413','NA','US',42.19,-73.35,6260);
INSERT INTO `zips` VALUES (1235,'STANDARD',0x00,'Hinsdale','Peru',NULL,'MA','Berkshire County','America/New_York','413','NA','US',42.42,-73.07,2580);
INSERT INTO `zips` VALUES (1236,'STANDARD',0x00,'Housatonic',NULL,NULL,'MA','Berkshire County','America/New_York','413','NA','US',42.27,-73.38,1420);
INSERT INTO `zips` VALUES (1237,'STANDARD',0x00,'Lanesboro','Hancock, New Ashford','Lanesborough','MA','Berkshire County','America/New_York','413','NA','US',42.51,-73.22,2720);
INSERT INTO `zips` VALUES (1238,'STANDARD',0x00,'Lee',NULL,'W Becket','MA','Berkshire County','America/New_York','413','NA','US',42.30,-73.25,5160);
INSERT INTO `zips` VALUES (1240,'STANDARD',0x00,'Lenox',NULL,NULL,'MA','Berkshire County','America/New_York','413','NA','US',42.35,-73.28,4110);
INSERT INTO `zips` VALUES (1242,'PO BOX',0x00,'Lenox Dale',NULL,NULL,'MA','Berkshire County','America/New_York','413','NA','US',42.33,-73.25,559);
INSERT INTO `zips` VALUES (1243,'PO BOX',0x00,'Middlefield',NULL,NULL,'MA','Hampshire County','America/New_York','413','NA','US',42.35,-73.01,315);
INSERT INTO `zips` VALUES (1244,'PO BOX',0x00,'Mill River',NULL,NULL,'MA','Berkshire County','America/New_York','413','NA','US',42.12,-73.26,328);
INSERT INTO `zips` VALUES (1245,'STANDARD',0x00,'Monterey','West Otis',NULL,'MA','Berkshire County','America/New_York','413','NA','US',42.18,-73.21,680);
INSERT INTO `zips` VALUES (1247,'STANDARD',0x00,'North Adams','Clarksburg, Florida','N Adams, No Adams','MA','Berkshire County','America/New_York','413','NA','US',42.68,-73.11,11710);
INSERT INTO `zips` VALUES (1252,'PO BOX',0x00,'North Egremont','N Egremont','No Egremont','MA','Berkshire County','America/New_York','413','NA','US',42.17,-73.44,281);
INSERT INTO `zips` VALUES (1253,'STANDARD',0x00,'Otis',NULL,'Cold Spring, North Otis','MA','Berkshire County','America/New_York','413','NA','US',42.21,-73.11,740);
INSERT INTO `zips` VALUES (1254,'STANDARD',0x00,'Richmond',NULL,NULL,'MA','Berkshire County','America/New_York','413','NA','US',42.37,-73.36,1060);
INSERT INTO `zips` VALUES (1255,'STANDARD',0x00,'Sandisfield',NULL,'South Sandisfield','MA','Berkshire County','America/New_York','413','NA','US',42.11,-73.13,600);
INSERT INTO `zips` VALUES (1256,'STANDARD',0x00,'Savoy',NULL,NULL,'MA','Berkshire County','America/New_York','413','NA','US',42.56,-73.02,590);
INSERT INTO `zips` VALUES (1257,'STANDARD',0x00,'Sheffield',NULL,NULL,'MA','Berkshire County','America/New_York','413','NA','US',42.10,-73.34,2080);
INSERT INTO `zips` VALUES (1258,'STANDARD',0x00,'South Egremont','Mount Washington, Mt Washington, S Egremont','So Egremont','MA','Berkshire County','America/New_York','413','NA','US',42.11,-73.46,600);
INSERT INTO `zips` VALUES (1259,'STANDARD',0x00,'Southfield',NULL,NULL,'MA','Berkshire County','America/New_York','413','NA','US',42.07,-73.23,410);
INSERT INTO `zips` VALUES (1260,'PO BOX',0x00,'South Lee',NULL,NULL,'MA','Berkshire County','America/New_York','413','NA','US',42.30,-73.34,254);
INSERT INTO `zips` VALUES (1262,'PO BOX',0x00,'Stockbridge',NULL,NULL,'MA','Berkshire County','America/New_York','413','NA','US',42.30,-73.32,1342);
INSERT INTO `zips` VALUES (1263,'UNIQUE',0x00,'Stockbridge',NULL,'Assoc Of Marian Helpers, Marian Helpers, Marion Fathers','MA','Berkshire County','America/New_York','413','NA','US',42.30,-73.32,0);
INSERT INTO `zips` VALUES (1264,'PO BOX',0x00,'Tyringham','Lee',NULL,'MA','Berkshire County','America/New_York','413','NA','US',42.24,-73.19,198);
INSERT INTO `zips` VALUES (1266,'STANDARD',0x00,'West Stockbridge','Alford, W Stockbridge','Interlaken, West Stockbridge Center','MA','Berkshire County','America/New_York','413','NA','US',42.31,-73.39,1240);
INSERT INTO `zips` VALUES (1267,'STANDARD',0x00,'Williamstown',NULL,'Williamstn, Wmstown','MA','Berkshire County','America/New_York','413','NA','US',42.70,-73.20,5170);
INSERT INTO `zips` VALUES (1270,'STANDARD',0x00,'Windsor',NULL,'East Windsor','MA','Berkshire County','America/New_York','413','NA','US',42.51,-73.04,710);
INSERT INTO `zips` VALUES (1301,'STANDARD',0x00,'Greenfield','Leyden',NULL,'MA','Franklin County','America/New_York','413','NA','US',42.58,-72.59,14160);
INSERT INTO `zips` VALUES (1302,'PO BOX',0x00,'Greenfield',NULL,NULL,'MA','Franklin County','America/New_York','413','NA','US',42.58,-72.59,502);
INSERT INTO `zips` VALUES (1330,'STANDARD',0x00,'Ashfield',NULL,'South Ashfield','MA','Franklin County','America/New_York','413','NA','US',42.53,-72.78,1310);
INSERT INTO `zips` VALUES (1331,'STANDARD',0x00,'Athol','Phillipston','Royalston','MA','Worcester County','America/New_York','978','NA','US',42.59,-72.23,11210);
INSERT INTO `zips` VALUES (1337,'STANDARD',0x00,'Bernardston','Leyden',NULL,'MA','Franklin County','America/New_York','413','NA','US',42.66,-72.55,2440);
INSERT INTO `zips` VALUES (1338,'STANDARD',0x00,'Buckland',NULL,NULL,'MA','Franklin County','America/New_York','413','NA','US',42.57,-72.82,210);
INSERT INTO `zips` VALUES (1339,'STANDARD',0x00,'Charlemont','Hawley','West Hawley','MA','Franklin County','America/New_York',NULL,'NA','US',42.63,-72.88,1240);
INSERT INTO `zips` VALUES (1340,'STANDARD',0x00,'Colrain','Shattuckville',NULL,'MA','Franklin County','America/New_York','413','NA','US',42.66,-72.68,1600);
INSERT INTO `zips` VALUES (1341,'STANDARD',0x00,'Conway',NULL,NULL,'MA','Franklin County','America/New_York','413','NA','US',42.51,-72.68,1560);
INSERT INTO `zips` VALUES (1342,'STANDARD',0x00,'Deerfield',NULL,'East Deerfield, West Deerfield','MA','Franklin County','America/New_York','413','NA','US',42.55,-72.60,1340);
INSERT INTO `zips` VALUES (1343,'STANDARD',0x00,'Drury',NULL,'Charlemont','MA','Berkshire County','America/New_York','413','NA','US',42.66,-72.98,153);
INSERT INTO `zips` VALUES (1344,'STANDARD',0x00,'Erving',NULL,'Farley, Stoneville','MA','Franklin County','America/New_York',NULL,'NA','US',42.60,-72.40,1390);
INSERT INTO `zips` VALUES (1346,'STANDARD',0x00,'Heath','Charlemont',NULL,'MA','Franklin County','America/New_York','413','NA','US',42.66,-72.83,340);
INSERT INTO `zips` VALUES (1347,'PO BOX',0x00,'Lake Pleasant',NULL,NULL,'MA','Franklin County','America/New_York',NULL,'NA','US',42.56,-72.52,165);
INSERT INTO `zips` VALUES (1349,'STANDARD',0x00,'Millers Falls',NULL,'Turners Falls','MA','Franklin County','America/New_York','413','NA','US',42.56,-72.48,720);
INSERT INTO `zips` VALUES (1350,'PO BOX',0x00,'Monroe Bridge','Monroe',NULL,'MA','Franklin County','America/New_York',NULL,'NA','US',42.72,-72.98,35);
INSERT INTO `zips` VALUES (1351,'STANDARD',0x00,'Montague',NULL,NULL,'MA','Franklin County','America/New_York','413','NA','US',42.53,-72.53,2110);
INSERT INTO `zips` VALUES (1354,'STANDARD',0x00,'Gill','Mount Hermon, Mt Hermon, Northfield Mount Hermon, Northfield Mt Hermon',NULL,'MA','Franklin County','America/New_York','413','NA','US',42.62,-72.51,1450);
INSERT INTO `zips` VALUES (1355,'STANDARD',0x00,'New Salem',NULL,'Orange','MA','Franklin County','America/New_York','978','NA','US',42.50,-72.33,860);
INSERT INTO `zips` VALUES (1360,'STANDARD',0x00,'Northfield',NULL,'N Field, No Field','MA','Franklin County','America/New_York','413','NA','US',42.70,-72.43,2680);
INSERT INTO `zips` VALUES (1364,'STANDARD',0x00,'Orange','Warwick','Blissville, Eagleville, Lake Mattawa, N Orange, New Salem, North Orange','MA','Franklin County','America/New_York','978','NA','US',42.59,-72.30,6140);
INSERT INTO `zips` VALUES (1366,'STANDARD',0x00,'Petersham',NULL,NULL,'MA','Worcester County','America/New_York','978','NA','US',42.48,-72.18,1090);
INSERT INTO `zips` VALUES (1367,'STANDARD',0x00,'Rowe',NULL,'Hoosac Tunnel, Zoar','MA','Franklin County','America/New_York','413','NA','US',42.70,-72.90,440);
INSERT INTO `zips` VALUES (1368,'STANDARD',0x00,'Royalston','S Royalston','Athol','MA','Worcester County','America/New_York','978','NA','US',42.68,-72.18,1050);
INSERT INTO `zips` VALUES (1370,'STANDARD',0x00,'Shelburne Falls','Shelburne Fls','Baptist Corner, East Charlemont, Shelburne','MA','Franklin County','America/New_York','413','NA','US',42.60,-72.74,3510);
INSERT INTO `zips` VALUES (1373,'STANDARD',0x00,'South Deerfield','S Deerfield','So Deerfield, Whately','MA','Franklin County','America/New_York','413','NA','US',42.47,-72.59,4070);
INSERT INTO `zips` VALUES (1375,'STANDARD',0x00,'Sunderland',NULL,NULL,'MA','Franklin County','America/New_York','413','NA','US',42.46,-72.58,2880);
INSERT INTO `zips` VALUES (1376,'STANDARD',0x00,'Turners Falls',NULL,NULL,'MA','Franklin County','America/New_York','413','NA','US',42.59,-72.55,4320);
INSERT INTO `zips` VALUES (1378,'STANDARD',0x00,'Warwick','Orange',NULL,'MA','Franklin County','America/New_York','978','NA','US',42.68,-72.33,630);
INSERT INTO `zips` VALUES (1379,'STANDARD',0x00,'Wendell',NULL,NULL,'MA','Franklin County','America/New_York',NULL,'NA','US',42.55,-72.40,750);
INSERT INTO `zips` VALUES (1380,'STANDARD',0x00,'Wendell Depot',NULL,NULL,'MA','Franklin County','America/New_York',NULL,'NA','US',42.58,-72.37,81);
INSERT INTO `zips` VALUES (1420,'STANDARD',0x00,'Fitchburg',NULL,NULL,'MA','Worcester County','America/New_York','978,508','NA','US',42.58,-71.81,33060);
INSERT INTO `zips` VALUES (1430,'STANDARD',0x00,'Ashburnham',NULL,'South Ashburnham','MA','Worcester County','America/New_York','978','NA','US',42.63,-71.90,5730);
INSERT INTO `zips` VALUES (1431,'STANDARD',0x00,'Ashby',NULL,NULL,'MA','Middlesex County','America/New_York','978','NA','US',42.68,-71.81,2860);
INSERT INTO `zips` VALUES (1432,'STANDARD',0x00,'Ayer',NULL,'Devens, Fort Devens, Ft Devens','MA','Middlesex County','America/New_York','978','NA','US',42.56,-71.58,7280);
INSERT INTO `zips` VALUES (1434,'STANDARD',0x00,'Devens','Ayer',NULL,'MA','Worcester County','America/New_York',NULL,'NA','US',42.53,-71.61,440);
INSERT INTO `zips` VALUES (1436,'STANDARD',0x00,'Baldwinville',NULL,'Otter River','MA','Worcester County','America/New_York',NULL,'NA','US',42.60,-72.07,2580);
INSERT INTO `zips` VALUES (1438,'PO BOX',0x00,'East Templeton','E Templeton',NULL,'MA','Worcester County','America/New_York',NULL,'NA','US',42.56,-72.03,704);
INSERT INTO `zips` VALUES (1440,'STANDARD',0x00,'Gardner',NULL,NULL,'MA','Worcester County','America/New_York','978','NA','US',42.58,-71.98,16400);
INSERT INTO `zips` VALUES (1441,'UNIQUE',0x00,'Westminster',NULL,'Gardner, Tyco','MA','Worcester County','America/New_York','351','NA','US',42.58,-71.98,0);
INSERT INTO `zips` VALUES (1450,'STANDARD',0x00,'Groton',NULL,NULL,'MA','Middlesex County','America/New_York','978','NA','US',42.60,-71.57,10720);
INSERT INTO `zips` VALUES (1451,'STANDARD',0x00,'Harvard',NULL,NULL,'MA','Worcester County','America/New_York','978','NA','US',42.50,-71.58,5190);
INSERT INTO `zips` VALUES (1452,'STANDARD',0x00,'Hubbardston',NULL,NULL,'MA','Worcester County','America/New_York','978','NA','US',42.48,-72.01,4150);
INSERT INTO `zips` VALUES (1453,'STANDARD',0x00,'Leominster',NULL,NULL,'MA','Worcester County','America/New_York','508,978','NA','US',42.51,-71.77,37880);
INSERT INTO `zips` VALUES (1460,'STANDARD',0x00,'Littleton',NULL,'Pingryville','MA','Middlesex County','America/New_York','508,781,978','NA','US',42.53,-71.47,9360);
INSERT INTO `zips` VALUES (1462,'STANDARD',0x00,'Lunenburg',NULL,NULL,'MA','Worcester County','America/New_York','508,978','NA','US',42.59,-71.72,10460);
INSERT INTO `zips` VALUES (1463,'STANDARD',0x00,'Pepperell',NULL,'E Pepperell, East Pepperell','MA','Middlesex County','America/New_York','351,978','NA','US',42.66,-71.58,10880);
INSERT INTO `zips` VALUES (1464,'STANDARD',0x00,'Shirley','Shirley Center, Shirley Ctr',NULL,'MA','Middlesex County','America/New_York','978','NA','US',42.54,-71.65,5680);
INSERT INTO `zips` VALUES (1467,'PO BOX',0x00,'Still River',NULL,NULL,'MA','Worcester County','America/New_York',NULL,'NA','US',42.49,-71.61,318);
INSERT INTO `zips` VALUES (1468,'STANDARD',0x00,'Templeton',NULL,NULL,'MA','Worcester County','America/New_York','978','NA','US',42.55,-72.06,3940);
INSERT INTO `zips` VALUES (1469,'STANDARD',0x00,'Townsend',NULL,NULL,'MA','Middlesex County','America/New_York','978','NA','US',42.66,-71.70,6770);
INSERT INTO `zips` VALUES (1470,'UNIQUE',0x00,'Groton',NULL,'New England Business','MA','Middlesex County','America/New_York','351','NA','US',42.60,-71.57,0);
INSERT INTO `zips` VALUES (1471,'UNIQUE',0x00,'Groton',NULL,'New England Business Svc Inc','MA','Middlesex County','America/New_York','351','NA','US',42.60,-71.57,0);
INSERT INTO `zips` VALUES (1472,'PO BOX',0x00,'West Groton',NULL,'W Groton','MA','Middlesex County','America/New_York',NULL,'NA','US',42.61,-71.62,352);
INSERT INTO `zips` VALUES (1473,'STANDARD',0x00,'Westminster',NULL,NULL,'MA','Worcester County','America/New_York','978','NA','US',42.55,-71.90,7330);
INSERT INTO `zips` VALUES (1474,'STANDARD',0x00,'West Townsend','Townsend, W Townsend',NULL,'MA','Middlesex County','America/New_York','978','NA','US',42.66,-71.74,1830);
INSERT INTO `zips` VALUES (1475,'STANDARD',0x00,'Winchendon',NULL,NULL,'MA','Worcester County','America/New_York','978','NA','US',42.68,-72.04,8680);
INSERT INTO `zips` VALUES (1477,'PO BOX',0x00,'Winchendon Springs','Winchdon Spgs',NULL,'MA','Worcester County','America/New_York','351','NA','US',42.69,-72.01,214);
INSERT INTO `zips` VALUES (1501,'STANDARD',0x00,'Auburn',NULL,NULL,'MA','Worcester County','America/New_York','508,774','NA','US',42.20,-71.83,15470);
INSERT INTO `zips` VALUES (1503,'STANDARD',0x00,'Berlin',NULL,NULL,'MA','Worcester County','America/New_York','978','NA','US',42.38,-71.63,2840);
INSERT INTO `zips` VALUES (1504,'STANDARD',0x00,'Blackstone',NULL,'E Blackstone, East Blackstone, Millerville','MA','Worcester County','America/New_York','508,774','NA','US',42.04,-71.53,8380);
INSERT INTO `zips` VALUES (1505,'STANDARD',0x00,'Boylston',NULL,'Morningdale','MA','Worcester County','America/New_York','508,774','NA','US',42.35,-71.73,4310);
INSERT INTO `zips` VALUES (1506,'STANDARD',0x00,'Brookfield',NULL,NULL,'MA','Worcester County','America/New_York','413','NA','US',42.21,-72.10,3050);
INSERT INTO `zips` VALUES (1507,'STANDARD',0x00,'Charlton',NULL,NULL,'MA','Worcester County','America/New_York','508,774','NA','US',42.13,-71.96,11860);
INSERT INTO `zips` VALUES (1508,'PO BOX',0x00,'Charlton City',NULL,'Richardson Corners','MA','Worcester County','America/New_York',NULL,'NA','US',42.13,-71.96,958);
INSERT INTO `zips` VALUES (1509,'PO BOX',0x00,'Charlton Depot','Charlton Dept, Charlton Dpt',NULL,'MA','Worcester County','America/New_York',NULL,'NA','US',42.13,-71.96,52);
INSERT INTO `zips` VALUES (1510,'STANDARD',0x00,'Clinton',NULL,NULL,'MA','Worcester County','America/New_York','978','NA','US',42.41,-71.68,13050);
INSERT INTO `zips` VALUES (1515,'STANDARD',0x00,'East Brookfield','E Brookfield',NULL,'MA','Worcester County','America/New_York','508,774','NA','US',42.22,-72.04,2140);
INSERT INTO `zips` VALUES (1516,'STANDARD',0x00,'Douglas','East Douglas',NULL,'MA','Worcester County','America/New_York','508,774','NA','US',42.05,-71.73,8430);
INSERT INTO `zips` VALUES (1517,'PO BOX',0x00,'East Princeton','E Princeton',NULL,'MA','Worcester County','America/New_York',NULL,'NA','US',42.46,-71.89,0);
INSERT INTO `zips` VALUES (1518,'STANDARD',0x00,'Fiskdale','Sturbridge',NULL,'MA','Worcester County','America/New_York','774,508','NA','US',42.12,-72.11,2990);
INSERT INTO `zips` VALUES (1519,'STANDARD',0x00,'Grafton',NULL,'Hassanamisco Indian Reservat','MA','Worcester County','America/New_York','508,774','NA','US',42.20,-71.68,6990);
INSERT INTO `zips` VALUES (1520,'STANDARD',0x00,'Holden',NULL,NULL,'MA','Worcester County','America/New_York','508,774','NA','US',42.35,-71.85,15210);
INSERT INTO `zips` VALUES (1521,'STANDARD',0x00,'Holland','Fiskdale','Halland','MA','Hampden County','America/New_York','508','NA','US',42.05,-72.15,2180);
INSERT INTO `zips` VALUES (1522,'STANDARD',0x00,'Jefferson',NULL,NULL,'MA','Worcester County','America/New_York',NULL,'NA','US',42.38,-71.87,3240);
INSERT INTO `zips` VALUES (1523,'STANDARD',0x00,'Lancaster',NULL,'North Lancaster','MA','Worcester County','America/New_York',NULL,'NA','US',42.45,-71.66,5860);
INSERT INTO `zips` VALUES (1524,'STANDARD',0x00,'Leicester',NULL,NULL,'MA','Worcester County','America/New_York','508,774','NA','US',42.25,-71.90,6030);
INSERT INTO `zips` VALUES (1525,'PO BOX',0x00,'Linwood',NULL,NULL,'MA','Worcester County','America/New_York',NULL,'NA','US',42.11,-71.63,1058);
INSERT INTO `zips` VALUES (1526,'PO BOX',0x00,'Manchaug',NULL,NULL,'MA','Worcester County','America/New_York',NULL,'NA','US',42.12,-71.76,534);
INSERT INTO `zips` VALUES (1527,'STANDARD',0x00,'Millbury',NULL,'East Millbury','MA','Worcester County','America/New_York','508,774','NA','US',42.19,-71.76,12130);
INSERT INTO `zips` VALUES (1529,'STANDARD',0x00,'Millville',NULL,NULL,'MA','Worcester County','America/New_York',NULL,'NA','US',42.03,-71.58,2950);
INSERT INTO `zips` VALUES (1531,'STANDARD',0x00,'New Braintree',NULL,NULL,'MA','Worcester County','America/New_York','508,774','NA','US',42.31,-72.13,950);
INSERT INTO `zips` VALUES (1532,'STANDARD',0x00,'Northborough',NULL,'Northboro','MA','Worcester County','America/New_York','508,774','NA','US',42.31,-71.64,15300);
INSERT INTO `zips` VALUES (1534,'STANDARD',0x00,'Northbridge',NULL,'Rockdale','MA','Worcester County','America/New_York',NULL,'NA','US',42.15,-71.65,5830);
INSERT INTO `zips` VALUES (1535,'STANDARD',0x00,'North Brookfield','N Brookfield',NULL,'MA','Worcester County','America/New_York','508,774','NA','US',42.27,-72.08,4150);
INSERT INTO `zips` VALUES (1536,'STANDARD',0x00,'North Grafton',NULL,'N Grafton, No Grafton','MA','Worcester County','America/New_York',NULL,'NA','US',42.23,-71.69,6880);
INSERT INTO `zips` VALUES (1537,'STANDARD',0x00,'North Oxford',NULL,NULL,'MA','Worcester County','America/New_York','508','NA','US',42.16,-71.88,2000);
INSERT INTO `zips` VALUES (1538,'PO BOX',0x00,'North Uxbridge','N Uxbridge',NULL,'MA','Worcester County','America/New_York',NULL,'NA','US',42.05,-71.64,584);
INSERT INTO `zips` VALUES (1540,'STANDARD',0x00,'Oxford',NULL,NULL,'MA','Worcester County','America/New_York','508,774','NA','US',42.11,-71.87,10400);
INSERT INTO `zips` VALUES (1541,'STANDARD',0x00,'Princeton',NULL,NULL,'MA','Worcester County','America/New_York','978','NA','US',42.45,-71.86,3480);
INSERT INTO `zips` VALUES (1542,'STANDARD',0x00,'Rochdale',NULL,NULL,'MA','Worcester County','America/New_York',NULL,'NA','US',42.20,-71.90,2100);
INSERT INTO `zips` VALUES (1543,'STANDARD',0x00,'Rutland',NULL,NULL,'MA','Worcester County','America/New_York','508,774','NA','US',42.36,-71.95,8260);
INSERT INTO `zips` VALUES (1545,'STANDARD',0x00,'Shrewsbury',NULL,'Edgemere','MA','Worcester County','America/New_York','508,774','NA','US',42.30,-71.71,36050);
INSERT INTO `zips` VALUES (1546,'UNIQUE',0x00,'Shrewsbury',NULL,'Central Mass P & D Ctr','MA','Worcester County','America/New_York','508','NA','US',42.30,-71.71,0);
INSERT INTO `zips` VALUES (1550,'STANDARD',0x00,'Southbridge',NULL,'Globe Village, Sandersdale','MA','Worcester County','America/New_York','508,774','NA','US',42.08,-72.03,14970);
INSERT INTO `zips` VALUES (1560,'STANDARD',0x00,'South Grafton',NULL,'Saundersville','MA','Worcester County','America/New_York',NULL,'NA','US',42.17,-71.68,4330);
INSERT INTO `zips` VALUES (1561,'PO BOX',0x00,'South Lancaster','S Lancaster','So Lancaster','MA','Worcester County','America/New_York',NULL,'NA','US',42.44,-71.69,1100);
INSERT INTO `zips` VALUES (1562,'STANDARD',0x00,'Spencer',NULL,'Lambs Grove','MA','Worcester County','America/New_York','508,774','NA','US',42.24,-71.99,10260);
INSERT INTO `zips` VALUES (1564,'STANDARD',0x00,'Sterling',NULL,'Sterling Junction','MA','Worcester County','America/New_York','978','NA','US',42.43,-71.75,7630);
INSERT INTO `zips` VALUES (1566,'STANDARD',0x00,'Sturbridge',NULL,NULL,'MA','Worcester County','America/New_York','508,774','NA','US',42.09,-72.06,6400);
INSERT INTO `zips` VALUES (1568,'STANDARD',0x00,'Upton',NULL,'W Upton, West Upton','MA','Worcester County','America/New_York','508,774','NA','US',42.17,-71.61,7420);
INSERT INTO `zips` VALUES (1569,'STANDARD',0x00,'Uxbridge',NULL,NULL,'MA','Worcester County','America/New_York','774,508','NA','US',42.08,-71.60,12610);
INSERT INTO `zips` VALUES (1570,'STANDARD',0x00,'Webster','Dudley Hill',NULL,'MA','Worcester County','America/New_York','508,774','NA','US',42.04,-71.87,14980);
INSERT INTO `zips` VALUES (1571,'STANDARD',0x00,'Dudley',NULL,NULL,'MA','Worcester County','America/New_York',NULL,'NA','US',42.05,-71.93,10050);
INSERT INTO `zips` VALUES (1580,'UNIQUE',0x00,'Westborough',NULL,'Emc, Westboro','MA','Worcester County','America/New_York','508','NA','US',42.26,-71.61,0);
INSERT INTO `zips` VALUES (1581,'STANDARD',0x00,'Westborough',NULL,'Westboro','MA','Worcester County','America/New_York','508,774','NA','US',42.26,-71.61,19460);
INSERT INTO `zips` VALUES (1582,'UNIQUE',0x00,'Westborough',NULL,'National Grid Co, Westboro','MA','Worcester County','America/New_York','508','NA','US',42.26,-71.61,0);
INSERT INTO `zips` VALUES (1583,'STANDARD',0x00,'West Boylston',NULL,'Oakdale, Westboylston','MA','Worcester County','America/New_York',NULL,'NA','US',42.36,-71.78,6550);
INSERT INTO `zips` VALUES (1585,'STANDARD',0x00,'West Brookfield','W Brookfield','Westbrookfield','MA','Worcester County','America/New_York','508,413','NA','US',42.23,-72.14,3710);
INSERT INTO `zips` VALUES (1586,'PO BOX',0x00,'West Millbury','Millbury',NULL,'MA','Worcester County','America/New_York',NULL,'NA','US',42.19,-71.76,0);
INSERT INTO `zips` VALUES (1588,'STANDARD',0x00,'Whitinsville','Linwood',NULL,'MA','Worcester County','America/New_York','508,774','NA','US',42.11,-71.67,9200);
INSERT INTO `zips` VALUES (1590,'STANDARD',0x00,'Sutton','Wilkinsonvile, Wilkinsonville',NULL,'MA','Worcester County','America/New_York',NULL,'NA','US',42.15,-71.76,8700);
INSERT INTO `zips` VALUES (1601,'PO BOX',0x00,'Worcester',NULL,NULL,'MA','Worcester County','America/New_York','508','NA','US',42.26,-71.80,322);
INSERT INTO `zips` VALUES (1602,'STANDARD',0x00,'Worcester',NULL,'West Side','MA','Worcester County','America/New_York',NULL,'NA','US',42.27,-71.85,19650);
INSERT INTO `zips` VALUES (1603,'STANDARD',0x00,'Worcester',NULL,'Webster Square','MA','Worcester County','America/New_York','508','NA','US',42.24,-71.84,17400);
INSERT INTO `zips` VALUES (1604,'STANDARD',0x00,'Worcester',NULL,NULL,'MA','Worcester County','America/New_York','508','NA','US',42.25,-71.77,31060);
INSERT INTO `zips` VALUES (1605,'STANDARD',0x00,'Worcester',NULL,NULL,'MA','Worcester County','America/New_York','508','NA','US',42.29,-71.79,21860);
INSERT INTO `zips` VALUES (1606,'STANDARD',0x00,'Worcester',NULL,'Greendale','MA','Worcester County','America/New_York','508','NA','US',42.32,-71.80,18160);
INSERT INTO `zips` VALUES (1607,'STANDARD',0x00,'Worcester',NULL,NULL,'MA','Worcester County','America/New_York','508','NA','US',42.23,-71.79,7320);
INSERT INTO `zips` VALUES (1608,'STANDARD',0x00,'Worcester',NULL,NULL,'MA','Worcester County','America/New_York','774,978,508','NA','US',42.26,-71.80,2790);
INSERT INTO `zips` VALUES (1609,'STANDARD',0x00,'Worcester',NULL,NULL,'MA','Worcester County','America/New_York','508,774,978','NA','US',42.29,-71.83,12910);
INSERT INTO `zips` VALUES (1610,'STANDARD',0x00,'Worcester',NULL,NULL,'MA','Worcester County','America/New_York','508,774','NA','US',42.25,-71.81,16530);
INSERT INTO `zips` VALUES (1611,'STANDARD',0x00,'Cherry Valley',NULL,NULL,'MA','Worcester County','America/New_York',NULL,'NA','US',42.23,-71.87,2100);
INSERT INTO `zips` VALUES (1612,'STANDARD',0x00,'Paxton','Worcester',NULL,'MA','Worcester County','America/New_York',NULL,'NA','US',42.31,-71.93,4310);
INSERT INTO `zips` VALUES (1613,'PO BOX',0x00,'Worcester',NULL,NULL,'MA','Worcester County','America/New_York','508','NA','US',42.26,-71.80,1369);
INSERT INTO `zips` VALUES (1614,'PO BOX',0x00,'Worcester',NULL,NULL,'MA','Worcester County','America/New_York','508','NA','US',42.26,-71.80,68);
INSERT INTO `zips` VALUES (1615,'PO BOX',0x00,'Worcester',NULL,NULL,'MA','Worcester County','America/New_York','508','NA','US',42.26,-71.80,13);
INSERT INTO `zips` VALUES (1653,'UNIQUE',0x00,'Worcester',NULL,'Allmerica','MA','Worcester County','America/New_York','508','NA','US',42.26,-71.80,0);
INSERT INTO `zips` VALUES (1654,'UNIQUE',0x00,'Worcester',NULL,'Verizon','MA','Worcester County','America/New_York','508','NA','US',42.26,-71.80,0);
INSERT INTO `zips` VALUES (1655,'STANDARD',0x00,'Worcester',NULL,NULL,'MA','Worcester County','America/New_York','508,774,978','NA','US',42.26,-71.80,0);
INSERT INTO `zips` VALUES (1701,'STANDARD',0x00,'Framingham',NULL,'Framingham Center, Framingham So, Saxonville','MA','Middlesex County','America/New_York','508,774,781,978','NA','US',42.30,-71.43,30670);
INSERT INTO `zips` VALUES (1702,'STANDARD',0x00,'Framingham',NULL,NULL,'MA','Middlesex County','America/New_York','508,774,781,978','NA','US',42.28,-71.44,28760);
INSERT INTO `zips` VALUES (1703,'PO BOX',0x00,'Framingham',NULL,NULL,'MA','Middlesex County','America/New_York','508','NA','US',42.30,-71.43,160);
INSERT INTO `zips` VALUES (1704,'PO BOX',0x00,'Framingham',NULL,NULL,'MA','Middlesex County','America/New_York','508','NA','US',42.30,-71.43,385);
INSERT INTO `zips` VALUES (1705,'PO BOX',0x00,'Framingham',NULL,NULL,'MA','Middlesex County','America/New_York','508','NA','US',42.30,-71.43,71);
INSERT INTO `zips` VALUES (1718,'STANDARD',0x00,'Village Of Nagog Woods','Acton, Vlg Nagog Wds','Vlg Of Nagog Woods','MA','Middlesex County','America/New_York','508,978,781','NA','US',42.52,-71.43,630);
INSERT INTO `zips` VALUES (1719,'STANDARD',0x00,'Boxborough','Acton, Boxboro',NULL,'MA','Middlesex County','America/New_York','508,978','NA','US',42.50,-71.50,5020);
INSERT INTO `zips` VALUES (1720,'STANDARD',0x00,'Acton',NULL,'W Acton, West Acton','MA','Middlesex County','America/New_York','781,508,978','NA','US',42.48,-71.46,22280);
INSERT INTO `zips` VALUES (1721,'STANDARD',0x00,'Ashland',NULL,NULL,'MA','Middlesex County','America/New_York',NULL,'NA','US',42.25,-71.46,16840);
INSERT INTO `zips` VALUES (1730,'STANDARD',0x00,'Bedford',NULL,NULL,'MA','Middlesex County','America/New_York','781,857','NA','US',42.48,-71.26,13570);
INSERT INTO `zips` VALUES (1731,'STANDARD',0x00,'Hanscom Afb','Bedford',NULL,'MA','Middlesex County','America/New_York','857,781','NA','US',42.46,-71.28,2450);
INSERT INTO `zips` VALUES (1740,'STANDARD',0x00,'Bolton',NULL,NULL,'MA','Worcester County','America/New_York','978','NA','US',42.43,-71.60,5230);
INSERT INTO `zips` VALUES (1741,'STANDARD',0x00,'Carlisle',NULL,NULL,'MA','Middlesex County','America/New_York',NULL,'NA','US',42.53,-71.35,5220);
INSERT INTO `zips` VALUES (1742,'STANDARD',0x00,'Concord',NULL,'W Concord, West Concord','MA','Middlesex County','America/New_York','978','NA','US',42.45,-71.35,17170);
INSERT INTO `zips` VALUES (1745,'STANDARD',0x00,'Fayville','Southborough','Southboro','MA','Worcester County','America/New_York',NULL,'NA','US',42.29,-71.50,430);
INSERT INTO `zips` VALUES (1746,'STANDARD',0x00,'Holliston',NULL,NULL,'MA','Middlesex County','America/New_York','508,774','NA','US',42.20,-71.43,14290);
INSERT INTO `zips` VALUES (1747,'STANDARD',0x00,'Hopedale',NULL,NULL,'MA','Worcester County','America/New_York',NULL,'NA','US',42.12,-71.54,5830);
INSERT INTO `zips` VALUES (1748,'STANDARD',0x00,'Hopkinton',NULL,NULL,'MA','Middlesex County','America/New_York','508,774','NA','US',42.22,-71.52,16150);
INSERT INTO `zips` VALUES (1749,'STANDARD',0x00,'Hudson',NULL,NULL,'MA','Middlesex County','America/New_York','508,978','NA','US',42.39,-71.56,18590);
INSERT INTO `zips` VALUES (1752,'STANDARD',0x00,'Marlborough',NULL,'Marlboro','MA','Middlesex County','America/New_York','508,774,978','NA','US',42.34,-71.54,36750);
INSERT INTO `zips` VALUES (1754,'STANDARD',0x00,'Maynard',NULL,NULL,'MA','Middlesex County','America/New_York','978','NA','US',42.42,-71.45,9910);
INSERT INTO `zips` VALUES (1756,'STANDARD',0x00,'Mendon',NULL,NULL,'MA','Worcester County','America/New_York',NULL,'NA','US',42.10,-71.55,6060);
INSERT INTO `zips` VALUES (1757,'STANDARD',0x00,'Milford',NULL,NULL,'MA','Worcester County','America/New_York','508,774','NA','US',42.14,-71.51,26360);
INSERT INTO `zips` VALUES (1760,'STANDARD',0x00,'Natick',NULL,'N Natick, No Natick, North Natick, S Natick, So Natick, South Natick','MA','Middlesex County','America/New_York','508,774','NA','US',42.28,-71.35,34350);
INSERT INTO `zips` VALUES (1770,'STANDARD',0x00,'Sherborn',NULL,NULL,'MA','Middlesex County','America/New_York',NULL,'NA','US',42.23,-71.36,4310);
INSERT INTO `zips` VALUES (1772,'STANDARD',0x00,'Southborough',NULL,'Southboro','MA','Worcester County','America/New_York','508,978','NA','US',42.30,-71.51,9970);
INSERT INTO `zips` VALUES (1773,'STANDARD',0x00,'Lincoln',NULL,'Lincoln Center','MA','Middlesex County','America/New_York','781','NA','US',42.41,-71.30,5340);
INSERT INTO `zips` VALUES (1775,'STANDARD',0x00,'Stow',NULL,NULL,'MA','Middlesex County','America/New_York',NULL,'NA','US',42.43,-71.50,7030);
INSERT INTO `zips` VALUES (1776,'STANDARD',0x00,'Sudbury',NULL,'N Sudbury, North Sudbury','MA','Middlesex County','America/New_York','978','NA','US',42.36,-71.40,18410);
INSERT INTO `zips` VALUES (1778,'STANDARD',0x00,'Wayland',NULL,'Cochituate','MA','Middlesex County','America/New_York','508,774','NA','US',42.36,-71.36,13910);
INSERT INTO `zips` VALUES (1784,'PO BOX',0x00,'Woodville',NULL,NULL,'MA','Middlesex County','America/New_York',NULL,'NA','US',42.23,-71.55,150);
INSERT INTO `zips` VALUES (1801,'STANDARD',0x00,'Woburn',NULL,NULL,'MA','Middlesex County','America/New_York','339,617,781,978','NA','US',42.48,-71.15,37270);
INSERT INTO `zips` VALUES (1803,'STANDARD',0x00,'Burlington',NULL,NULL,'MA','Middlesex County','America/New_York','339,781','NA','US',42.50,-71.20,24740);
INSERT INTO `zips` VALUES (1805,'UNIQUE',0x00,'Burlington',NULL,'Lahey Clinic Med Ctr','MA','Middlesex County','America/New_York','339','NA','US',42.50,-71.20,0);
INSERT INTO `zips` VALUES (1806,'UNIQUE',0x01,'Woburn','At&t',NULL,'MA','Middlesex County','America/New_York','339','NA','US',42.47,-71.15,0);
INSERT INTO `zips` VALUES (1807,'UNIQUE',0x00,'Woburn',NULL,'National Grid','MA','Middlesex County','America/New_York','339','NA','US',42.48,-71.15,0);
INSERT INTO `zips` VALUES (1808,'UNIQUE',0x01,'Woburn',NULL,'At&t','MA','Middlesex County','America/New_York','339','NA','US',42.50,-71.12,0);
INSERT INTO `zips` VALUES (1810,'STANDARD',0x00,'Andover',NULL,NULL,'MA','Essex County','America/New_York','508,978','NA','US',42.65,-71.14,33590);
INSERT INTO `zips` VALUES (1812,'UNIQUE',0x00,'Andover',NULL,'Internal Revenue Service','MA','Essex County','America/New_York','351','NA','US',42.65,-71.14,0);
INSERT INTO `zips` VALUES (1813,'UNIQUE',0x00,'Woburn',NULL,'Mellon Financial Services','MA','Middlesex County','America/New_York','339','NA','US',42.48,-71.15,0);
INSERT INTO `zips` VALUES (1815,'UNIQUE',0x00,'Woburn',NULL,'Bank Of America','MA','Middlesex County','America/New_York','339','NA','US',42.48,-71.15,0);
INSERT INTO `zips` VALUES (1821,'STANDARD',0x00,'Billerica',NULL,NULL,'MA','Middlesex County','America/New_York','508,978','NA','US',42.55,-71.26,29980);
INSERT INTO `zips` VALUES (1822,'PO BOX',0x00,'Billerica',NULL,NULL,'MA','Middlesex County','America/New_York','351','NA','US',42.55,-71.26,0);
INSERT INTO `zips` VALUES (1824,'STANDARD',0x00,'Chelmsford','Kates Corner, S Chelmsford',NULL,'MA','Middlesex County','America/New_York',NULL,'NA','US',42.59,-71.36,25110);
INSERT INTO `zips` VALUES (1826,'STANDARD',0x00,'Dracut',NULL,NULL,'MA','Middlesex County','America/New_York',NULL,'NA','US',42.68,-71.30,29730);
INSERT INTO `zips` VALUES (1827,'STANDARD',0x00,'Dunstable',NULL,NULL,'MA','Middlesex County','America/New_York',NULL,'NA','US',42.66,-71.48,3120);
INSERT INTO `zips` VALUES (1830,'STANDARD',0x00,'Haverhill',NULL,NULL,'MA','Essex County','America/New_York','978','NA','US',42.78,-71.08,21980);
INSERT INTO `zips` VALUES (1831,'PO BOX',0x00,'Haverhill',NULL,NULL,'MA','Essex County','America/New_York','351','NA','US',42.78,-71.08,992);
INSERT INTO `zips` VALUES (1832,'STANDARD',0x00,'Haverhill',NULL,NULL,'MA','Essex County','America/New_York','978','NA','US',42.79,-71.13,21090);
INSERT INTO `zips` VALUES (1833,'STANDARD',0x00,'Georgetown','Haverhill',NULL,'MA','Essex County','America/New_York','351,978','NA','US',42.73,-70.98,8180);
INSERT INTO `zips` VALUES (1834,'STANDARD',0x00,'Groveland',NULL,NULL,'MA','Essex County','America/New_York',NULL,'NA','US',42.75,-71.03,6340);
INSERT INTO `zips` VALUES (1835,'STANDARD',0x00,'Haverhill','Bradford, Ward Hill',NULL,'MA','Essex County','America/New_York','978','NA','US',42.75,-71.09,13130);
INSERT INTO `zips` VALUES (1840,'STANDARD',0x00,'Lawrence',NULL,NULL,'MA','Essex County','America/New_York','508,978','NA','US',42.71,-71.16,4200);
INSERT INTO `zips` VALUES (1841,'STANDARD',0x00,'Lawrence',NULL,NULL,'MA','Essex County','America/New_York','508,978','NA','US',42.71,-71.16,43940);
INSERT INTO `zips` VALUES (1842,'PO BOX',0x00,'Lawrence',NULL,NULL,'MA','Essex County','America/New_York','351','NA','US',42.70,-71.16,1297);
INSERT INTO `zips` VALUES (1843,'STANDARD',0x00,'Lawrence',NULL,'S Lawrence, South Lawrence','MA','Essex County','America/New_York','508,978','NA','US',42.70,-71.16,23800);
INSERT INTO `zips` VALUES (1844,'STANDARD',0x00,'Methuen',NULL,NULL,'MA','Essex County','America/New_York',NULL,'NA','US',42.74,-71.18,47210);
INSERT INTO `zips` VALUES (1845,'STANDARD',0x00,'North Andover',NULL,'N Andover','MA','Essex County','America/New_York',NULL,'NA','US',42.70,-71.11,28000);
INSERT INTO `zips` VALUES (1850,'STANDARD',0x00,'Lowell',NULL,NULL,'MA','Middlesex County','America/New_York','781,978','NA','US',42.66,-71.30,13690);
INSERT INTO `zips` VALUES (1851,'STANDARD',0x00,'Lowell',NULL,NULL,'MA','Middlesex County','America/New_York','781,978','NA','US',42.63,-71.32,29150);
INSERT INTO `zips` VALUES (1852,'STANDARD',0x00,'Lowell',NULL,NULL,'MA','Middlesex County','America/New_York','781,978','NA','US',42.63,-71.30,29580);
INSERT INTO `zips` VALUES (1853,'PO BOX',0x00,'Lowell',NULL,NULL,'MA','Middlesex County','America/New_York','351','NA','US',42.63,-71.32,1641);
INSERT INTO `zips` VALUES (1854,'STANDARD',0x00,'Lowell',NULL,NULL,'MA','Middlesex County','America/New_York','978','NA','US',42.65,-71.35,20260);
INSERT INTO `zips` VALUES (1860,'STANDARD',0x00,'Merrimac',NULL,NULL,'MA','Essex County','America/New_York','978','NA','US',42.83,-71.00,6110);
INSERT INTO `zips` VALUES (1862,'STANDARD',0x00,'North Billerica','N Billerica','Billerica','MA','Middlesex County','America/New_York','508,978','NA','US',42.58,-71.30,9070);
INSERT INTO `zips` VALUES (1863,'STANDARD',0x00,'North Chelmsford','N Chelmsford',NULL,'MA','Middlesex County','America/New_York',NULL,'NA','US',42.63,-71.39,8740);
INSERT INTO `zips` VALUES (1864,'STANDARD',0x00,'North Reading',NULL,'N Reading','MA','Middlesex County','America/New_York','978','NA','US',42.58,-71.08,15120);
INSERT INTO `zips` VALUES (1865,'PO BOX',0x00,'Nutting Lake',NULL,'Nuttings Lake','MA','Middlesex County','America/New_York',NULL,'NA','US',42.54,-71.25,397);
INSERT INTO `zips` VALUES (1866,'PO BOX',0x00,'Pinehurst',NULL,NULL,'MA','Middlesex County','America/New_York',NULL,'NA','US',42.53,-71.23,178);
INSERT INTO `zips` VALUES (1867,'STANDARD',0x00,'Reading',NULL,NULL,'MA','Middlesex County','America/New_York','781','NA','US',42.53,-71.10,24810);
INSERT INTO `zips` VALUES (1876,'STANDARD',0x00,'Tewksbury',NULL,NULL,'MA','Middlesex County','America/New_York',NULL,'NA','US',42.61,-71.23,28550);
INSERT INTO `zips` VALUES (1879,'STANDARD',0x00,'Tyngsboro',NULL,'Tyngsborough','MA','Middlesex County','America/New_York','978','NA','US',42.68,-71.41,11610);
INSERT INTO `zips` VALUES (1880,'STANDARD',0x00,'Wakefield',NULL,NULL,'MA','Middlesex County','America/New_York','339,781','NA','US',42.50,-71.06,25180);
INSERT INTO `zips` VALUES (1885,'PO BOX',0x00,'West Boxford',NULL,NULL,'MA','Essex County','America/New_York',NULL,'NA','US',42.67,-71.03,331);
INSERT INTO `zips` VALUES (1886,'STANDARD',0x00,'Westford',NULL,'Forge Village, Nabnasset','MA','Middlesex County','America/New_York','508,978','NA','US',42.58,-71.43,24190);
INSERT INTO `zips` VALUES (1887,'STANDARD',0x00,'Wilmington',NULL,NULL,'MA','Middlesex County','America/New_York','978','NA','US',42.55,-71.16,22520);
INSERT INTO `zips` VALUES (1888,'PO BOX',0x00,'Woburn',NULL,NULL,'MA','Middlesex County','America/New_York','339','NA','US',42.48,-71.15,242);
INSERT INTO `zips` VALUES (1889,'UNIQUE',0x00,'North Reading',NULL,'Massachusetts District, N Reading','MA','Middlesex County','America/New_York','351','NA','US',42.56,-71.06,0);
INSERT INTO `zips` VALUES (1890,'STANDARD',0x00,'Winchester',NULL,NULL,'MA','Middlesex County','America/New_York','781','NA','US',42.45,-71.14,22310);
INSERT INTO `zips` VALUES (1899,'UNIQUE',0x00,'Andover',NULL,'Bar Coded I R S','MA','Essex County','America/New_York','351','NA','US',42.65,-71.14,0);
INSERT INTO `zips` VALUES (1901,'STANDARD',0x00,'Lynn',NULL,NULL,'MA','Essex County','America/New_York','339,781','NA','US',42.46,-70.95,1440);
INSERT INTO `zips` VALUES (1902,'STANDARD',0x00,'Lynn',NULL,NULL,'MA','Essex County','America/New_York','339,781','NA','US',42.47,-70.94,40210);
INSERT INTO `zips` VALUES (1903,'PO BOX',0x00,'Lynn',NULL,NULL,'MA','Essex County','America/New_York','339','NA','US',42.47,-70.96,1107);
INSERT INTO `zips` VALUES (1904,'STANDARD',0x00,'Lynn','East Lynn',NULL,'MA','Essex County','America/New_York','508,978','NA','US',42.47,-70.96,18340);
INSERT INTO `zips` VALUES (1905,'STANDARD',0x00,'Lynn','West Lynn',NULL,'MA','Essex County','America/New_York','617,339,781','NA','US',42.47,-70.98,23830);
INSERT INTO `zips` VALUES (1906,'STANDARD',0x00,'Saugus',NULL,NULL,'MA','Essex County','America/New_York','339,617,781','NA','US',42.46,-71.01,25580);
INSERT INTO `zips` VALUES (1907,'STANDARD',0x00,'Swampscott',NULL,NULL,'MA','Essex County','America/New_York',NULL,'NA','US',42.47,-70.91,13970);
INSERT INTO `zips` VALUES (1908,'STANDARD',0x00,'Nahant',NULL,NULL,'MA','Essex County','America/New_York',NULL,'NA','US',42.43,-70.93,3200);
INSERT INTO `zips` VALUES (1910,'UNIQUE',0x00,'Lynn',NULL,'General Elec Co','MA','Essex County','America/New_York','339','NA','US',42.47,-70.96,0);
INSERT INTO `zips` VALUES (1913,'STANDARD',0x00,'Amesbury',NULL,NULL,'MA','Essex County','America/New_York','978','NA','US',42.85,-70.92,15190);
INSERT INTO `zips` VALUES (1915,'STANDARD',0x00,'Beverly',NULL,'Beverly Farms','MA','Essex County','America/New_York','508,978','NA','US',42.57,-70.87,34410);
INSERT INTO `zips` VALUES (1921,'STANDARD',0x00,'Boxford',NULL,NULL,'MA','Essex County','America/New_York',NULL,'NA','US',42.67,-70.98,7940);
INSERT INTO `zips` VALUES (1922,'STANDARD',0x00,'Byfield','Newbury',NULL,'MA','Essex County','America/New_York',NULL,'NA','US',42.75,-70.93,3110);
INSERT INTO `zips` VALUES (1923,'STANDARD',0x00,'Danvers',NULL,NULL,'MA','Essex County','America/New_York','351,978','NA','US',42.57,-70.95,25830);
INSERT INTO `zips` VALUES (1929,'STANDARD',0x00,'Essex',NULL,NULL,'MA','Essex County','America/New_York','978','NA','US',42.63,-70.77,3350);
INSERT INTO `zips` VALUES (1930,'STANDARD',0x00,'Gloucester',NULL,'Magnolia','MA','Essex County','America/New_York','508,978','NA','US',42.62,-70.66,25490);
INSERT INTO `zips` VALUES (1931,'PO BOX',0x00,'Gloucester',NULL,NULL,'MA','Essex County','America/New_York','351','NA','US',42.62,-70.66,554);
INSERT INTO `zips` VALUES (1936,'PO BOX',0x00,'Hamilton',NULL,NULL,'MA','Essex County','America/New_York',NULL,'NA','US',42.61,-70.86,361);
INSERT INTO `zips` VALUES (1937,'PO BOX',0x00,'Hathorne',NULL,NULL,'MA','Essex County','America/New_York',NULL,'NA','US',42.59,-70.98,375);
INSERT INTO `zips` VALUES (1938,'STANDARD',0x00,'Ipswich',NULL,NULL,'MA','Essex County','America/New_York','978','NA','US',42.67,-70.83,12860);
INSERT INTO `zips` VALUES (1940,'STANDARD',0x00,'Lynnfield',NULL,'South Lynnfield','MA','Essex County','America/New_York','781','NA','US',42.53,-71.04,12880);
INSERT INTO `zips` VALUES (1944,'STANDARD',0x00,'Manchester','Manchester By The Sea',NULL,'MA','Essex County','America/New_York','508,978','NA','US',42.56,-70.76,5110);
INSERT INTO `zips` VALUES (1945,'STANDARD',0x00,'Marblehead',NULL,'Mhead','MA','Essex County','America/New_York','781','NA','US',42.50,-70.85,20070);
INSERT INTO `zips` VALUES (1949,'STANDARD',0x00,'Middleton',NULL,NULL,'MA','Essex County','America/New_York',NULL,'NA','US',42.60,-71.01,8180);
INSERT INTO `zips` VALUES (1950,'STANDARD',0x00,'Newburyport',NULL,'Plum Island','MA','Essex County','America/New_York','978','NA','US',42.81,-70.88,16750);
INSERT INTO `zips` VALUES (1951,'STANDARD',0x00,'Newbury','Newburyport','Plum Island','MA','Essex County','America/New_York','978','NA','US',42.77,-70.85,3390);
INSERT INTO `zips` VALUES (1952,'STANDARD',0x00,'Salisbury','Salisbury Bch, Salisbury Beach',NULL,'MA','Essex County','America/New_York',NULL,'NA','US',42.83,-70.84,7300);
INSERT INTO `zips` VALUES (1960,'STANDARD',0x00,'Peabody',NULL,'West Peabody','MA','Essex County','America/New_York','508,978','NA','US',42.53,-70.97,47880);
INSERT INTO `zips` VALUES (1961,'PO BOX',0x00,'Peabody',NULL,NULL,'MA','Essex County','America/New_York','351','NA','US',42.53,-70.97,315);
INSERT INTO `zips` VALUES (1965,'PO BOX',0x00,'Prides Crossing','Prides Crssng',NULL,'MA','Essex County','America/New_York','351','NA','US',42.56,-70.86,428);
INSERT INTO `zips` VALUES (1966,'STANDARD',0x00,'Rockport',NULL,'Pigeon Cove','MA','Essex County','America/New_York','508,351,978','NA','US',42.64,-70.61,6380);
INSERT INTO `zips` VALUES (1969,'STANDARD',0x00,'Rowley',NULL,NULL,'MA','Essex County','America/New_York','978','NA','US',42.72,-70.89,6010);
INSERT INTO `zips` VALUES (1970,'STANDARD',0x00,'Salem',NULL,NULL,'MA','Essex County','America/New_York','508,978','NA','US',42.51,-70.90,36170);
INSERT INTO `zips` VALUES (1971,'PO BOX',0x00,'Salem',NULL,NULL,'MA','Essex County','America/New_York','351','NA','US',42.51,-70.89,356);
INSERT INTO `zips` VALUES (1982,'STANDARD',0x00,'South Hamilton','S Hamilton','Hamilton','MA','Essex County','America/New_York','978','NA','US',42.62,-70.86,6980);
INSERT INTO `zips` VALUES (1983,'STANDARD',0x00,'Topsfield',NULL,NULL,'MA','Essex County','America/New_York','978','NA','US',42.63,-70.95,6370);
INSERT INTO `zips` VALUES (1984,'STANDARD',0x00,'Wenham',NULL,NULL,'MA','Essex County','America/New_York',NULL,'NA','US',42.60,-70.88,3710);
INSERT INTO `zips` VALUES (1985,'STANDARD',0x00,'West Newbury',NULL,NULL,'MA','Essex County','America/New_York','978','NA','US',42.80,-71.00,4380);
INSERT INTO `zips` VALUES (2018,'PO BOX',0x00,'Accord','Hingham','Norwell','MA','Plymouth County','America/New_York','339','NA','US',42.17,-70.88,123);
INSERT INTO `zips` VALUES (2019,'STANDARD',0x00,'Bellingham',NULL,NULL,'MA','Norfolk County','America/New_York','508,774','NA','US',42.09,-71.47,15890);
INSERT INTO `zips` VALUES (2020,'PO BOX',0x00,'Brant Rock',NULL,'Marshfield','MA','Plymouth County','America/New_York',NULL,'NA','US',42.08,-70.64,759);
INSERT INTO `zips` VALUES (2021,'STANDARD',0x00,'Canton',NULL,NULL,'MA','Norfolk County','America/New_York','339,781','NA','US',42.15,-71.13,21930);
INSERT INTO `zips` VALUES (2025,'STANDARD',0x00,'Cohasset',NULL,NULL,'MA','Norfolk County','America/New_York','339,781','NA','US',42.23,-70.80,8100);
INSERT INTO `zips` VALUES (2026,'STANDARD',0x00,'Dedham',NULL,NULL,'MA','Norfolk County','America/New_York','617,781','NA','US',42.24,-71.17,23210);
INSERT INTO `zips` VALUES (2027,'PO BOX',0x00,'Dedham',NULL,NULL,'MA','Norfolk County','America/New_York','339','NA','US',42.24,-71.17,183);
INSERT INTO `zips` VALUES (2030,'STANDARD',0x00,'Dover',NULL,NULL,'MA','Norfolk County','America/New_York','508,774','NA','US',42.24,-71.27,5800);
INSERT INTO `zips` VALUES (2031,'STANDARD',0x01,'East Mansfield','Mansfield, E Mansfield',NULL,'MA','Bristol County','America/New_York','508','NA','US',42.02,-71.17,0);
INSERT INTO `zips` VALUES (2032,'STANDARD',0x00,'East Walpole',NULL,'E Walpole, Walpole','MA','Norfolk County','America/New_York','508,774,339,781','NA','US',42.15,-71.21,4580);
INSERT INTO `zips` VALUES (2035,'STANDARD',0x00,'Foxboro','Foxborough',NULL,'MA','Norfolk County','America/New_York','781,508,774','NA','US',42.06,-71.24,16830);
INSERT INTO `zips` VALUES (2038,'STANDARD',0x00,'Franklin',NULL,NULL,'MA','Norfolk County','America/New_York','508,774','NA','US',42.08,-71.38,30920);
INSERT INTO `zips` VALUES (2040,'PO BOX',0x00,'Greenbush','Scituate',NULL,'MA','Plymouth County','America/New_York','339','NA','US',42.19,-70.76,122);
INSERT INTO `zips` VALUES (2041,'PO BOX',0x00,'Green Harbor',NULL,'Marshfield','MA','Plymouth County','America/New_York',NULL,'NA','US',42.10,-70.71,843);
INSERT INTO `zips` VALUES (2043,'STANDARD',0x00,'Hingham',NULL,'Accord','MA','Plymouth County','America/New_York','339,781','NA','US',42.23,-70.88,23080);
INSERT INTO `zips` VALUES (2044,'UNIQUE',0x00,'Hingham',NULL,'Shared Firm Zip Code','MA','Plymouth County','America/New_York','339','NA','US',42.23,-70.88,0);
INSERT INTO `zips` VALUES (2045,'STANDARD',0x00,'Hull',NULL,'Nantasket Beach','MA','Plymouth County','America/New_York','781','NA','US',42.30,-70.90,9080);
INSERT INTO `zips` VALUES (2047,'PO BOX',0x00,'Humarock',NULL,'Marshfield','MA','Plymouth County','America/New_York','339','NA','US',42.13,-70.69,649);
INSERT INTO `zips` VALUES (2048,'STANDARD',0x00,'Mansfield',NULL,NULL,'MA','Bristol County','America/New_York','508,774','NA','US',42.02,-71.21,23100);
INSERT INTO `zips` VALUES (2050,'STANDARD',0x00,'Marshfield',NULL,NULL,'MA','Plymouth County','America/New_York','339,781','NA','US',42.09,-70.70,22550);
INSERT INTO `zips` VALUES (2051,'PO BOX',0x00,'Marshfield Hills','Marshfld Hls','Marshfield','MA','Plymouth County','America/New_York',NULL,'NA','US',42.15,-70.73,752);
INSERT INTO `zips` VALUES (2052,'STANDARD',0x00,'Medfield',NULL,NULL,'MA','Norfolk County','America/New_York','508,774','NA','US',42.18,-71.30,12690);
INSERT INTO `zips` VALUES (2053,'STANDARD',0x00,'Medway',NULL,NULL,'MA','Norfolk County','America/New_York','508,774','NA','US',42.16,-71.43,12820);
INSERT INTO `zips` VALUES (2054,'STANDARD',0x00,'Millis',NULL,NULL,'MA','Norfolk County','America/New_York','508,774','NA','US',42.16,-71.35,7840);
INSERT INTO `zips` VALUES (2055,'PO BOX',0x00,'Minot','Scituate',NULL,'MA','Plymouth County','America/New_York','339','NA','US',42.19,-70.76,77);
INSERT INTO `zips` VALUES (2056,'STANDARD',0x00,'Norfolk',NULL,NULL,'MA','Norfolk County','America/New_York',NULL,'NA','US',42.11,-71.31,9820);
INSERT INTO `zips` VALUES (2059,'PO BOX',0x00,'North Marshfield','N Marshfield','Marshfield','MA','Plymouth County','America/New_York',NULL,'NA','US',42.10,-70.71,363);
INSERT INTO `zips` VALUES (2060,'PO BOX',0x00,'North Scituate','N Scituate, Scituate',NULL,'MA','Plymouth County','America/New_York','339','NA','US',42.21,-70.76,237);
INSERT INTO `zips` VALUES (2061,'STANDARD',0x00,'Norwell',NULL,NULL,'MA','Plymouth County','America/New_York','339,781','NA','US',42.16,-70.78,10930);
INSERT INTO `zips` VALUES (2062,'STANDARD',0x00,'Norwood',NULL,NULL,'MA','Norfolk County','America/New_York','339,781','NA','US',42.18,-71.19,27500);
INSERT INTO `zips` VALUES (2065,'PO BOX',0x00,'Ocean Bluff','Marshfield',NULL,'MA','Plymouth County','America/New_York',NULL,'NA','US',42.10,-70.71,447);
INSERT INTO `zips` VALUES (2066,'STANDARD',0x00,'Scituate',NULL,'Scituate Center, Scituate Harbor','MA','Plymouth County','America/New_York','339,781','NA','US',42.18,-70.73,17470);
INSERT INTO `zips` VALUES (2067,'STANDARD',0x00,'Sharon',NULL,NULL,'MA','Norfolk County','America/New_York','339,781','NA','US',42.11,-71.18,18210);
INSERT INTO `zips` VALUES (2070,'PO BOX',0x00,'Sheldonville',NULL,'Wrentham','MA','Norfolk County','America/New_York',NULL,'NA','US',42.05,-71.35,182);
INSERT INTO `zips` VALUES (2071,'STANDARD',0x00,'South Walpole',NULL,'S Walpole, Walpole','MA','Norfolk County','America/New_York','508,774,781','NA','US',42.10,-71.27,1010);
INSERT INTO `zips` VALUES (2072,'STANDARD',0x00,'Stoughton',NULL,NULL,'MA','Norfolk County','America/New_York','781','NA','US',42.11,-71.10,26350);
INSERT INTO `zips` VALUES (2081,'STANDARD',0x00,'Walpole',NULL,NULL,'MA','Norfolk County','America/New_York','781,508,774','NA','US',42.13,-71.24,18890);
INSERT INTO `zips` VALUES (2090,'STANDARD',0x00,'Westwood',NULL,'Islington','MA','Norfolk County','America/New_York','339,617,781','NA','US',42.21,-71.21,15350);
INSERT INTO `zips` VALUES (2093,'STANDARD',0x00,'Wrentham',NULL,NULL,'MA','Norfolk County','America/New_York','508,774','NA','US',42.06,-71.33,10820);
INSERT INTO `zips` VALUES (2108,'STANDARD',0x00,'Boston',NULL,NULL,'MA','Suffolk County','America/New_York','617,857,339','NA','US',42.36,-71.06,4050);
INSERT INTO `zips` VALUES (2109,'STANDARD',0x00,'Boston',NULL,NULL,'MA','Suffolk County','America/New_York','617','NA','US',42.37,-71.05,3980);
INSERT INTO `zips` VALUES (2110,'STANDARD',0x00,'Boston',NULL,NULL,'MA','Suffolk County','America/New_York','508,774,617,781,857,978','NA','US',42.36,-71.05,3320);
INSERT INTO `zips` VALUES (2111,'STANDARD',0x00,'Boston',NULL,NULL,'MA','Suffolk County','America/New_York','617,781,978,339,857','NA','US',42.35,-71.06,5720);
INSERT INTO `zips` VALUES (2112,'PO BOX',0x00,'Boston',NULL,NULL,'MA','Suffolk County','America/New_York','617','NA','US',42.35,-71.06,574);
INSERT INTO `zips` VALUES (2113,'STANDARD',0x00,'Boston',NULL,NULL,'MA','Suffolk County','America/New_York','781,617','NA','US',42.37,-71.06,5320);
INSERT INTO `zips` VALUES (2114,'STANDARD',0x00,'Boston',NULL,NULL,'MA','Suffolk County','America/New_York','617,339,857','NA','US',42.36,-71.07,9350);
INSERT INTO `zips` VALUES (2115,'STANDARD',0x00,'Boston',NULL,NULL,'MA','Suffolk County','America/New_York','617,857','NA','US',42.34,-71.10,9810);
INSERT INTO `zips` VALUES (2116,'STANDARD',0x00,'Boston',NULL,NULL,'MA','Suffolk County','America/New_York','508,617,781,978,857','NA','US',42.35,-71.08,15420);
INSERT INTO `zips` VALUES (2117,'PO BOX',0x00,'Boston',NULL,NULL,'MA','Suffolk County','America/New_York','617','NA','US',42.35,-71.06,443);
INSERT INTO `zips` VALUES (2118,'STANDARD',0x00,'Boston','Roxbury',NULL,'MA','Suffolk County','America/New_York','857,617','NA','US',42.34,-71.07,20190);
INSERT INTO `zips` VALUES (2119,'STANDARD',0x00,'Roxbury','Boston',NULL,'MA','Suffolk County','America/New_York','617,857','NA','US',42.32,-71.09,22430);
INSERT INTO `zips` VALUES (2120,'STANDARD',0x00,'Roxbury Crossing','Boston, Mission Hill, Roxbury, Roxbury Xing',NULL,'MA','Suffolk County','America/New_York','857,617','NA','US',42.33,-71.10,7730);
INSERT INTO `zips` VALUES (2121,'STANDARD',0x00,'Dorchester','Boston, Grove Hall',NULL,'MA','Suffolk County','America/New_York','617,781,857','NA','US',42.31,-71.09,23300);
INSERT INTO `zips` VALUES (2122,'STANDARD',0x00,'Dorchester','Boston',NULL,'MA','Suffolk County','America/New_York','781,617,857','NA','US',42.29,-71.04,21000);
INSERT INTO `zips` VALUES (2123,'PO BOX',0x00,'Boston',NULL,NULL,'MA','Suffolk County','America/New_York','617','NA','US',42.35,-71.06,449);
INSERT INTO `zips` VALUES (2124,'STANDARD',0x00,'Dorchester Center','Boston, Dorchester, Dorchestr Ctr',NULL,'MA','Suffolk County','America/New_York','617','NA','US',42.29,-71.07,43850);
INSERT INTO `zips` VALUES (2125,'STANDARD',0x00,'Dorchester','Boston, Uphams Corner',NULL,'MA','Suffolk County','America/New_York','617,857,781','NA','US',42.32,-71.06,27860);
INSERT INTO `zips` VALUES (2126,'STANDARD',0x00,'Mattapan','Boston','Hyde Park','MA','Suffolk County','America/New_York','617','NA','US',42.27,-71.10,20900);
INSERT INTO `zips` VALUES (2127,'STANDARD',0x00,'Boston','South Boston','S Boston','MA','Suffolk County','America/New_York','617,774,781,978,857','NA','US',42.33,-71.04,29000);
INSERT INTO `zips` VALUES (2128,'STANDARD',0x00,'Boston','East Boston','E Boston','MA','Suffolk County','America/New_York','617,857','NA','US',42.36,-71.01,34440);
INSERT INTO `zips` VALUES (2129,'STANDARD',0x00,'Charlestown','Boston',NULL,'MA','Suffolk County','America/New_York','857,617,781','NA','US',42.38,-71.06,15910);
INSERT INTO `zips` VALUES (2130,'STANDARD',0x00,'Jamaica Plain','Boston',NULL,'MA','Suffolk County','America/New_York','617,857','NA','US',42.30,-71.11,30590);
INSERT INTO `zips` VALUES (2131,'STANDARD',0x00,'Roslindale','Boston',NULL,'MA','Suffolk County','America/New_York','617','NA','US',42.28,-71.13,27800);
INSERT INTO `zips` VALUES (2132,'STANDARD',0x00,'West Roxbury','Boston','W Roxbury','MA','Suffolk County','America/New_York','617,781','NA','US',42.28,-71.16,25130);
INSERT INTO `zips` VALUES (2133,'STANDARD',0x00,'Boston',NULL,NULL,'MA','Suffolk County','America/New_York','339,508,781,857,978,617','NA','US',42.35,-71.06,0);
INSERT INTO `zips` VALUES (2134,'STANDARD',0x00,'Allston','Boston',NULL,'MA','Suffolk County','America/New_York','617,857','NA','US',42.35,-71.13,13390);
INSERT INTO `zips` VALUES (2135,'STANDARD',0x00,'Brighton','Boston',NULL,'MA','Suffolk County','America/New_York','617,857','NA','US',42.35,-71.15,31660);
INSERT INTO `zips` VALUES (2136,'STANDARD',0x00,'Hyde Park','Boston, Readville',NULL,'MA','Suffolk County','America/New_York','617,857','NA','US',42.26,-71.13,32400);
INSERT INTO `zips` VALUES (2137,'PO BOX',0x00,'Readville','Boston, Hyde Park',NULL,'MA','Suffolk County','America/New_York','617','NA','US',42.25,-71.12,180);
INSERT INTO `zips` VALUES (2138,'STANDARD',0x00,'Cambridge',NULL,NULL,'MA','Middlesex County','America/New_York','508,617,857','NA','US',42.38,-71.14,22390);
INSERT INTO `zips` VALUES (2139,'STANDARD',0x00,'Cambridge',NULL,'Cambridgeport, Inman Square','MA','Middlesex County','America/New_York','339,781,978,508,617,857','NA','US',42.37,-71.11,26890);
INSERT INTO `zips` VALUES (2140,'STANDARD',0x00,'Cambridge','N Cambridge, North Cambridge','Porter Square','MA','Middlesex County','America/New_York','617','NA','US',42.39,-71.13,16870);
INSERT INTO `zips` VALUES (2141,'STANDARD',0x00,'Cambridge','E Cambridge, East Cambridge',NULL,'MA','Middlesex County','America/New_York','339,617,508,781,857,978','NA','US',42.37,-71.08,9740);
INSERT INTO `zips` VALUES (2142,'STANDARD',0x00,'Cambridge',NULL,'Kendall Square','MA','Middlesex County','America/New_York','508,781,857,978,339,617','NA','US',42.36,-71.08,2420);
INSERT INTO `zips` VALUES (2143,'STANDARD',0x00,'Somerville',NULL,'E Somerville, East Somerville','MA','Middlesex County','America/New_York','617,857','NA','US',42.38,-71.10,20890);
INSERT INTO `zips` VALUES (2144,'STANDARD',0x00,'Somerville','W Somerville, West Somerville',NULL,'MA','Middlesex County','America/New_York','617,857','NA','US',42.40,-71.12,20290);
INSERT INTO `zips` VALUES (2145,'STANDARD',0x00,'Somerville','Winter Hill',NULL,'MA','Middlesex County','America/New_York','617,857','NA','US',42.39,-71.10,22430);
INSERT INTO `zips` VALUES (2148,'STANDARD',0x00,'Malden',NULL,NULL,'MA','Middlesex County','America/New_York','857,339,617,781','NA','US',42.43,-71.05,54800);
INSERT INTO `zips` VALUES (2149,'STANDARD',0x00,'Everett',NULL,NULL,'MA','Middlesex County','America/New_York','617,857','NA','US',42.40,-71.05,39470);
INSERT INTO `zips` VALUES (2150,'STANDARD',0x00,'Chelsea',NULL,NULL,'MA','Suffolk County','America/New_York','617,857','NA','US',42.39,-71.03,32330);
INSERT INTO `zips` VALUES (2151,'STANDARD',0x00,'Revere',NULL,'Beachmont, Revere Beach','MA','Suffolk County','America/New_York','617,339,781','NA','US',42.41,-70.99,47170);
INSERT INTO `zips` VALUES (2152,'STANDARD',0x00,'Winthrop',NULL,NULL,'MA','Suffolk County','America/New_York','617,857','NA','US',42.37,-70.98,15680);
INSERT INTO `zips` VALUES (2153,'PO BOX',0x00,'Medford','Tufts Univ, Tufts University',NULL,'MA','Middlesex County','America/New_York','617','NA','US',42.42,-71.10,0);
INSERT INTO `zips` VALUES (2155,'STANDARD',0x00,'Medford',NULL,'W Medford, West Medford','MA','Middlesex County','America/New_York','617,339,781','NA','US',42.42,-71.10,50390);
INSERT INTO `zips` VALUES (2156,'PO BOX',0x00,'West Medford',NULL,'Medford, W Medford','MA','Middlesex County','America/New_York','339','NA','US',42.42,-71.13,20);
INSERT INTO `zips` VALUES (2163,'STANDARD',0x00,'Boston','Cambridge','Soldiers Field','MA','Suffolk County','America/New_York','339,781,978,508,617,857','NA','US',42.37,-71.12,630);
INSERT INTO `zips` VALUES (2169,'STANDARD',0x00,'Quincy',NULL,'Houghs Neck, Quincy Center, South Quincy, West Quincy','MA','Norfolk County','America/New_York','617,857','NA','US',42.26,-71.00,49210);
INSERT INTO `zips` VALUES (2170,'STANDARD',0x00,'Quincy','Wollaston',NULL,'MA','Norfolk County','America/New_York','617,857','NA','US',42.27,-71.02,18260);
INSERT INTO `zips` VALUES (2171,'STANDARD',0x00,'Quincy','North Quincy, Squantum','Marina Bay, N Quincy, No Quincy, Norfolk Downs','MA','Norfolk County','America/New_York','617,857','NA','US',42.29,-71.02,16680);
INSERT INTO `zips` VALUES (2176,'STANDARD',0x00,'Melrose',NULL,NULL,'MA','Middlesex County','America/New_York','339,617,781','NA','US',42.45,-71.05,26840);
INSERT INTO `zips` VALUES (2180,'STANDARD',0x00,'Stoneham',NULL,NULL,'MA','Middlesex County','America/New_York','617,781','NA','US',42.47,-71.09,21130);
INSERT INTO `zips` VALUES (2184,'STANDARD',0x00,'Braintree',NULL,'Braintree Highlands, Braintree Hld, E Braintree, East Braintree','MA','Norfolk County','America/New_York','339,617,781','NA','US',42.20,-71.00,36000);
INSERT INTO `zips` VALUES (2185,'PO BOX',0x00,'Braintree',NULL,'Braintree Phantom','MA','Norfolk County','America/New_York','339','NA','US',42.20,-71.00,257);
INSERT INTO `zips` VALUES (2186,'STANDARD',0x00,'Milton',NULL,'East Milton','MA','Norfolk County','America/New_York','617','NA','US',42.24,-71.08,26280);
INSERT INTO `zips` VALUES (2187,'PO BOX',0x00,'Milton Village','Milton Vlg',NULL,'MA','Norfolk County','America/New_York','617','NA','US',42.26,-71.08,63);
INSERT INTO `zips` VALUES (2188,'STANDARD',0x00,'Weymouth',NULL,'Weymouth Lndg','MA','Norfolk County','America/New_York','339,617,781','NA','US',42.20,-70.96,13740);
INSERT INTO `zips` VALUES (2189,'STANDARD',0x00,'East Weymouth','Weymouth',NULL,'MA','Norfolk County','America/New_York','339,617,781','NA','US',42.20,-70.94,13430);
INSERT INTO `zips` VALUES (2190,'STANDARD',0x00,'South Weymouth','S Weymouth, Weymouth','Weymouth Nas','MA','Norfolk County','America/New_York',NULL,'NA','US',42.17,-70.95,16180);
INSERT INTO `zips` VALUES (2191,'STANDARD',0x00,'North Weymouth','N Weymouth, Weymouth',NULL,'MA','Norfolk County','America/New_York','339,617,781','NA','US',42.24,-70.94,7750);
INSERT INTO `zips` VALUES (2196,'PO BOX',0x00,'Boston',NULL,NULL,'MA','Suffolk County','America/New_York','617','NA','US',42.35,-71.06,793);
INSERT INTO `zips` VALUES (2199,'STANDARD',0x00,'Boston',NULL,NULL,'MA','Suffolk County','America/New_York','339,857,978,508,617,781','NA','US',42.35,-71.08,1290);
INSERT INTO `zips` VALUES (2201,'UNIQUE',0x00,'Boston',NULL,'Boston City Hall','MA','Suffolk County','America/New_York','617','NA','US',42.35,-71.06,15);
INSERT INTO `zips` VALUES (2203,'STANDARD',0x00,'Boston',NULL,NULL,'MA','Suffolk County','America/New_York','857','NA','US',42.36,-71.06,0);
INSERT INTO `zips` VALUES (2204,'UNIQUE',0x00,'Boston',NULL,'Mass Tax, Massachusetts Tax','MA','Suffolk County','America/New_York','617','NA','US',42.35,-71.06,0);
INSERT INTO `zips` VALUES (2205,'PO BOX',0x00,'Boston',NULL,NULL,'MA','Suffolk County','America/New_York','617','NA','US',42.35,-71.06,1678);
INSERT INTO `zips` VALUES (2206,'UNIQUE',0x00,'Boston',NULL,'State Street Corporation','MA','Suffolk County','America/New_York','617','NA','US',42.35,-71.06,0);
INSERT INTO `zips` VALUES (2207,'UNIQUE',0x01,'Boston',NULL,'Bell Atlantic Telephone Co','MA','Suffolk County','America/New_York','617','NA','US',42.35,-71.05,0);
INSERT INTO `zips` VALUES (2210,'STANDARD',0x00,'Boston',NULL,NULL,'MA','Suffolk County','America/New_York','617,774,781,857,978','NA','US',42.35,-71.04,3140);
INSERT INTO `zips` VALUES (2211,'UNIQUE',0x00,'Boston',NULL,'Bank Of America','MA','Suffolk County','America/New_York','617','NA','US',42.35,-71.06,0);
INSERT INTO `zips` VALUES (2212,'UNIQUE',0x00,'Boston',NULL,'Bank Of America','MA','Suffolk County','America/New_York','617','NA','US',42.35,-71.06,0);
INSERT INTO `zips` VALUES (2215,'STANDARD',0x00,'Boston',NULL,'Boston University, Kenmore','MA','Suffolk County','America/New_York','617,857','NA','US',42.35,-71.10,7350);
INSERT INTO `zips` VALUES (2216,'UNIQUE',0x01,'Boston',NULL,'John Hancock P O Box 192','MA','Suffolk County','America/New_York','617','NA','US',42.34,-71.07,17);
INSERT INTO `zips` VALUES (2217,'UNIQUE',0x00,'Boston',NULL,'John Hancock P O Box 505','MA','Suffolk County','America/New_York','617','NA','US',42.35,-71.06,0);
INSERT INTO `zips` VALUES (2222,'STANDARD',0x00,'Boston',NULL,NULL,'MA','Suffolk County','America/New_York','857,617,781','NA','US',42.35,-71.06,0);
INSERT INTO `zips` VALUES (2228,'PO BOX',0x00,'East Boston','Boston',NULL,'MA','Suffolk County','America/New_York','617','NA','US',42.35,-71.06,93);
INSERT INTO `zips` VALUES (2238,'PO BOX',0x00,'Cambridge','Harvard Sq, Harvard Square',NULL,'MA','Middlesex County','America/New_York','617','NA','US',42.37,-71.11,534);
INSERT INTO `zips` VALUES (2239,'UNIQUE',0x01,'Cambridge','Com/energy Services',NULL,'MA','Middlesex County','America/New_York','617','NA','US',42.36,-71.10,0);
INSERT INTO `zips` VALUES (2241,'UNIQUE',0x00,'Boston',NULL,'Bank Of America, Fleet Bank Boston','MA','Suffolk County','America/New_York','617','NA','US',42.35,-71.06,0);
INSERT INTO `zips` VALUES (2266,'UNIQUE',0x00,'Boston',NULL,'Boston Financial Data Servic','MA','Suffolk County','America/New_York','617','NA','US',42.35,-71.06,0);
INSERT INTO `zips` VALUES (2269,'PO BOX',0x00,'Quincy',NULL,NULL,'MA','Norfolk County','America/New_York','617','NA','US',42.26,-71.00,478);
INSERT INTO `zips` VALUES (2283,'STANDARD',0x00,'Boston',NULL,NULL,'MA','Suffolk County','America/New_York','617,781,978,508,857','NA','US',42.35,-71.06,0);
INSERT INTO `zips` VALUES (2284,'STANDARD',0x00,'Boston',NULL,NULL,'MA','Suffolk County','America/New_York','508,617,781,857,978','NA','US',42.35,-71.06,0);
INSERT INTO `zips` VALUES (2293,'UNIQUE',0x00,'Boston',NULL,'Fidelity Service Company','MA','Suffolk County','America/New_York','617','NA','US',42.35,-71.06,0);
INSERT INTO `zips` VALUES (2295,'UNIQUE',0x00,'Boston',NULL,'John Hancock Mutual Ins','MA','Suffolk County','America/New_York','617','NA','US',42.35,-71.06,0);
INSERT INTO `zips` VALUES (2297,'UNIQUE',0x00,'Boston',NULL,'Cash Management','MA','Suffolk County','America/New_York','617','NA','US',42.35,-71.06,0);
INSERT INTO `zips` VALUES (2298,'PO BOX',0x00,'Boston',NULL,NULL,'MA','Suffolk County','America/New_York','617','NA','US',42.34,-71.05,0);
INSERT INTO `zips` VALUES (2301,'STANDARD',0x00,'Brockton',NULL,NULL,'MA','Plymouth County','America/New_York','508,774,781','NA','US',42.08,-71.02,56300);
INSERT INTO `zips` VALUES (2302,'STANDARD',0x00,'Brockton',NULL,NULL,'MA','Plymouth County','America/New_York','508,774,781','NA','US',42.09,-71.00,30790);
INSERT INTO `zips` VALUES (2303,'PO BOX',0x00,'Brockton',NULL,NULL,'MA','Plymouth County','America/New_York','508','NA','US',42.08,-71.02,1583);
INSERT INTO `zips` VALUES (2304,'PO BOX',0x00,'Brockton',NULL,NULL,'MA','Plymouth County','America/New_York','508','NA','US',42.08,-71.02,323);
INSERT INTO `zips` VALUES (2305,'PO BOX',0x00,'Brockton',NULL,NULL,'MA','Plymouth County','America/New_York','508','NA','US',42.08,-71.02,464);
INSERT INTO `zips` VALUES (2322,'STANDARD',0x00,'Avon',NULL,NULL,'MA','Norfolk County','America/New_York',NULL,'NA','US',42.13,-71.05,4220);
INSERT INTO `zips` VALUES (2324,'STANDARD',0x00,'Bridgewater',NULL,NULL,'MA','Plymouth County','America/New_York','508,774','NA','US',41.98,-70.97,21560);
INSERT INTO `zips` VALUES (2325,'UNIQUE',0x00,'Bridgewater',NULL,'Bridgewater State College','MA','Plymouth County','America/New_York','508','NA','US',41.98,-70.97,10);
INSERT INTO `zips` VALUES (2327,'PO BOX',0x00,'Bryantville',NULL,'Pembroke','MA','Plymouth County','America/New_York',NULL,'NA','US',42.06,-70.80,613);
INSERT INTO `zips` VALUES (2330,'STANDARD',0x00,'Carver',NULL,NULL,'MA','Plymouth County','America/New_York','508,774','NA','US',41.88,-70.76,10140);
INSERT INTO `zips` VALUES (2331,'PO BOX',0x00,'Duxbury',NULL,NULL,'MA','Plymouth County','America/New_York','339','NA','US',42.04,-70.67,1883);
INSERT INTO `zips` VALUES (2332,'STANDARD',0x00,'Duxbury',NULL,NULL,'MA','Plymouth County','America/New_York','781','NA','US',42.04,-70.67,14010);
INSERT INTO `zips` VALUES (2333,'STANDARD',0x00,'East Bridgewater','E Bridgewater, E Bridgewtr',NULL,'MA','Plymouth County','America/New_York','508,774','NA','US',42.03,-70.95,13540);
INSERT INTO `zips` VALUES (2334,'PO BOX',0x00,'Easton',NULL,NULL,'MA','Bristol County','America/New_York','508','NA','US',42.03,-71.10,375);
INSERT INTO `zips` VALUES (2337,'PO BOX',0x00,'Elmwood',NULL,'East Bridgewater','MA','Plymouth County','America/New_York',NULL,'NA','US',42.01,-70.96,119);
INSERT INTO `zips` VALUES (2338,'STANDARD',0x00,'Halifax',NULL,NULL,'MA','Plymouth County','America/New_York','339,781','NA','US',41.98,-70.86,7260);
INSERT INTO `zips` VALUES (2339,'STANDARD',0x00,'Hanover',NULL,'Assinippi, West Hanover','MA','Plymouth County','America/New_York','781','NA','US',42.11,-70.81,14110);
INSERT INTO `zips` VALUES (2340,'UNIQUE',0x00,'Hanover',NULL,'Wearguard','MA','Plymouth County','America/New_York','339','NA','US',42.11,-70.81,0);
INSERT INTO `zips` VALUES (2341,'STANDARD',0x00,'Hanson',NULL,NULL,'MA','Plymouth County','America/New_York','339,781','NA','US',42.06,-70.85,9710);
INSERT INTO `zips` VALUES (2343,'STANDARD',0x00,'Holbrook',NULL,NULL,'MA','Norfolk County','America/New_York',NULL,'NA','US',42.14,-71.00,10240);
INSERT INTO `zips` VALUES (2344,'UNIQUE',0x00,'Middleboro','Middleborough','Aetna Life & Casualty Co','MA','Plymouth County','America/New_York','508','NA','US',41.86,-70.90,0);
INSERT INTO `zips` VALUES (2345,'PO BOX',0x00,'Manomet',NULL,'Plymouth','MA','Plymouth County','America/New_York',NULL,'NA','US',41.92,-70.56,1470);
INSERT INTO `zips` VALUES (2346,'STANDARD',0x00,'Middleboro',NULL,'Middleborough','MA','Plymouth County','America/New_York','508,774','NA','US',41.86,-70.90,21370);
INSERT INTO `zips` VALUES (2347,'STANDARD',0x00,'Lakeville',NULL,'Middleboro','MA','Plymouth County','America/New_York','774','NA','US',41.85,-70.95,10570);
INSERT INTO `zips` VALUES (2348,'UNIQUE',0x00,'Lakeville','Middleboro, Middleborough','Lakeville Phantom, Talbots','MA','Plymouth County','America/New_York','508','NA','US',41.86,-70.90,0);
INSERT INTO `zips` VALUES (2349,'UNIQUE',0x00,'Middleboro','Middleborough','Ocean Spray','MA','Plymouth County','America/New_York','508','NA','US',41.86,-70.90,0);
INSERT INTO `zips` VALUES (2350,'PO BOX',0x00,'Monponsett',NULL,'Hanson','MA','Plymouth County','America/New_York',NULL,'NA','US',42.02,-70.84,494);
INSERT INTO `zips` VALUES (2351,'STANDARD',0x00,'Abington',NULL,NULL,'MA','Plymouth County','America/New_York',NULL,'NA','US',42.11,-70.95,15300);
INSERT INTO `zips` VALUES (2355,'PO BOX',0x00,'North Carver',NULL,'Carver, East Carver','MA','Plymouth County','America/New_York',NULL,'NA','US',41.91,-70.79,409);
INSERT INTO `zips` VALUES (2356,'STANDARD',0x00,'North Easton',NULL,'Easton, N Easton, No Easton','MA','Bristol County','America/New_York','508,774','NA','US',42.05,-71.10,12070);
INSERT INTO `zips` VALUES (2357,'UNIQUE',0x00,'North Easton','Stonehill Col, Stonehill College','Easton, Stonehill Coll','MA','Bristol County','America/New_York','508','NA','US',42.06,-71.08,14);
INSERT INTO `zips` VALUES (2358,'PO BOX',0x00,'North Pembroke','N Pembroke','Pembroke','MA','Plymouth County','America/New_York',NULL,'NA','US',42.09,-70.78,282);
INSERT INTO `zips` VALUES (2359,'STANDARD',0x00,'Pembroke',NULL,'East Pembroke','MA','Plymouth County','America/New_York','339,781','NA','US',42.06,-70.80,16950);
INSERT INTO `zips` VALUES (2360,'STANDARD',0x00,'Plymouth',NULL,'Cedarville','MA','Plymouth County','America/New_York','508,774','NA','US',41.95,-70.66,50690);
INSERT INTO `zips` VALUES (2361,'PO BOX',0x00,'Plymouth',NULL,NULL,'MA','Plymouth County','America/New_York','508','NA','US',41.95,-70.66,252);
INSERT INTO `zips` VALUES (2362,'PO BOX',0x00,'Plymouth',NULL,NULL,'MA','Plymouth County','America/New_York','508','NA','US',41.95,-70.66,981);
INSERT INTO `zips` VALUES (2364,'STANDARD',0x00,'Kingston',NULL,'Rocky Nook, Silver Lake','MA','Plymouth County','America/New_York','339,617,781','NA','US',41.99,-70.71,12300);
INSERT INTO `zips` VALUES (2366,'PO BOX',0x00,'South Carver',NULL,'Carver','MA','Plymouth County','America/New_York',NULL,'NA','US',41.85,-70.66,385);
INSERT INTO `zips` VALUES (2367,'STANDARD',0x00,'Plympton',NULL,NULL,'MA','Plymouth County','America/New_York',NULL,'NA','US',41.95,-70.81,2730);
INSERT INTO `zips` VALUES (2368,'STANDARD',0x00,'Randolph',NULL,NULL,'MA','Norfolk County','America/New_York','339,781','NA','US',42.17,-71.05,31010);
INSERT INTO `zips` VALUES (2370,'STANDARD',0x00,'Rockland',NULL,NULL,'MA','Plymouth County','America/New_York','339,781','NA','US',42.13,-70.91,16580);
INSERT INTO `zips` VALUES (2375,'STANDARD',0x00,'South Easton',NULL,'Easton, S Easton, So Easton','MA','Bristol County','America/New_York','508,774','NA','US',42.03,-71.10,9360);
INSERT INTO `zips` VALUES (2379,'STANDARD',0x00,'West Bridgewater','W Bridgewater',NULL,'MA','Plymouth County','America/New_York','508','NA','US',42.01,-71.00,6800);
INSERT INTO `zips` VALUES (2381,'PO BOX',0x00,'White Horse Beach','Wht Horse Bch','Plymouth','MA','Plymouth County','America/New_York',NULL,'NA','US',41.93,-70.59,383);
INSERT INTO `zips` VALUES (2382,'STANDARD',0x00,'Whitman',NULL,NULL,'MA','Plymouth County','America/New_York','781','NA','US',42.08,-70.93,14110);
INSERT INTO `zips` VALUES (2420,'STANDARD',0x00,'Lexington',NULL,NULL,'MA','Middlesex County','America/New_York','339,781','NA','US',42.46,-71.22,14980);
INSERT INTO `zips` VALUES (2421,'STANDARD',0x00,'Lexington',NULL,NULL,'MA','Middlesex County','America/New_York','339,781','NA','US',42.44,-71.23,17770);
INSERT INTO `zips` VALUES (2445,'STANDARD',0x00,'Brookline',NULL,NULL,'MA','Norfolk County','America/New_York','617,857','NA','US',42.32,-71.14,17920);
INSERT INTO `zips` VALUES (2446,'STANDARD',0x00,'Brookline',NULL,NULL,'MA','Norfolk County','America/New_York','617,857','NA','US',42.34,-71.12,23060);
INSERT INTO `zips` VALUES (2447,'PO BOX',0x00,'Brookline Village','Brookline Vlg','Brookline','MA','Norfolk County','America/New_York','617','NA','US',42.33,-71.13,120);
INSERT INTO `zips` VALUES (2451,'STANDARD',0x00,'Waltham','North Waltham',NULL,'MA','Middlesex County','America/New_York','339,617,781,508,978','NA','US',42.38,-71.24,16130);
INSERT INTO `zips` VALUES (2452,'STANDARD',0x00,'Waltham','North Waltham',NULL,'MA','Middlesex County','America/New_York','508,978,339,617,781','NA','US',42.39,-71.22,10840);
INSERT INTO `zips` VALUES (2453,'STANDARD',0x00,'Waltham','South Waltham',NULL,'MA','Middlesex County','America/New_York','508,978,339,617,781','NA','US',42.37,-71.24,23760);
INSERT INTO `zips` VALUES (2454,'PO BOX',0x00,'Waltham',NULL,NULL,'MA','Middlesex County','America/New_York','339','NA','US',42.38,-71.24,543);
INSERT INTO `zips` VALUES (2455,'PO BOX',0x00,'North Waltham',NULL,'Waltham','MA','Middlesex County','America/New_York','339','NA','US',42.39,-71.22,81);
INSERT INTO `zips` VALUES (2456,'PO BOX',0x00,'New Town',NULL,'Newton','MA','Middlesex County','America/New_York','617','NA','US',42.33,-71.20,28);
INSERT INTO `zips` VALUES (2457,'PO BOX',0x00,'Babson Park',NULL,'Wellesley','MA','Norfolk County','America/New_York','339','NA','US',42.30,-71.27,105);
INSERT INTO `zips` VALUES (2458,'STANDARD',0x00,'Newton','Newtonville','Riverside','MA','Middlesex County','America/New_York','617,857','NA','US',42.35,-71.19,11790);
INSERT INTO `zips` VALUES (2459,'STANDARD',0x00,'Newton Center','Newton, Newton Centre','Newton Cntr, Newton Ctr','MA','Middlesex County','America/New_York','617','NA','US',42.31,-71.19,17590);
INSERT INTO `zips` VALUES (2460,'STANDARD',0x00,'Newtonville','Newton',NULL,'MA','Middlesex County','America/New_York','617,857','NA','US',42.35,-71.20,8820);
INSERT INTO `zips` VALUES (2461,'STANDARD',0x00,'Newton Highlands','Newton, Newton Hlds',NULL,'MA','Middlesex County','America/New_York','617','NA','US',42.32,-71.21,7080);
INSERT INTO `zips` VALUES (2462,'STANDARD',0x00,'Newton Lower Falls','Newton, Newton L F, Newtonville',NULL,'MA','Middlesex County','America/New_York','617','NA','US',42.33,-71.26,1310);
INSERT INTO `zips` VALUES (2464,'STANDARD',0x00,'Newton Upper Falls','Newton, Newton U F',NULL,'MA','Middlesex County','America/New_York','617','NA','US',42.31,-71.22,2820);
INSERT INTO `zips` VALUES (2465,'STANDARD',0x00,'West Newton','Newton','W Newton','MA','Middlesex County','America/New_York','617,857','NA','US',42.35,-71.22,11730);
INSERT INTO `zips` VALUES (2466,'STANDARD',0x00,'Auburndale',NULL,'Newton','MA','Middlesex County','America/New_York','617','NA','US',42.34,-71.24,6570);
INSERT INTO `zips` VALUES (2467,'STANDARD',0x00,'Chestnut Hill','Boston Clg, Boston College','Brookline, Newton','MA','Norfolk County','America/New_York','857,617','NA','US',42.31,-71.16,14600);
INSERT INTO `zips` VALUES (2468,'STANDARD',0x00,'Waban',NULL,'Newton','MA','Middlesex County','America/New_York','617,781','NA','US',42.32,-71.23,5580);
INSERT INTO `zips` VALUES (2471,'PO BOX',0x00,'Watertown',NULL,'Watertown Financial','MA','Middlesex County','America/New_York','617','NA','US',42.36,-71.17,273);
INSERT INTO `zips` VALUES (2472,'STANDARD',0x00,'Watertown','E Watertown, East Watertown',NULL,'MA','Middlesex County','America/New_York','617,857','NA','US',42.37,-71.18,30280);
INSERT INTO `zips` VALUES (2474,'STANDARD',0x00,'Arlington','E Arlington, East Arlington',NULL,'MA','Middlesex County','America/New_York','339,781','NA','US',42.42,-71.16,26100);
INSERT INTO `zips` VALUES (2475,'PO BOX',0x00,'Arlington Heights','Arlington Hts','Arlington','MA','Middlesex County','America/New_York','339','NA','US',42.41,-71.18,19);
INSERT INTO `zips` VALUES (2476,'STANDARD',0x00,'Arlington',NULL,NULL,'MA','Middlesex County','America/New_York','339,781','NA','US',42.41,-71.16,16510);
INSERT INTO `zips` VALUES (2477,'UNIQUE',0x00,'Watertown',NULL,'Field Premium Inc','MA','Middlesex County','America/New_York','617','NA','US',42.36,-71.17,0);
INSERT INTO `zips` VALUES (2478,'STANDARD',0x00,'Belmont',NULL,NULL,'MA','Middlesex County','America/New_York','617,857','NA','US',42.39,-71.18,24720);
INSERT INTO `zips` VALUES (2479,'PO BOX',0x00,'Waverley',NULL,'Belmont','MA','Middlesex County','America/New_York',NULL,'NA','US',42.39,-71.17,39);
INSERT INTO `zips` VALUES (2481,'STANDARD',0x00,'Wellesley Hills','Wellesley, Wellesley Hls','Wellesley Fms','MA','Norfolk County','America/New_York','508,774,339,781','NA','US',42.31,-71.27,15000);
INSERT INTO `zips` VALUES (2482,'STANDARD',0x00,'Wellesley',NULL,NULL,'MA','Norfolk County','America/New_York','339,781,508,774','NA','US',42.29,-71.30,10250);
INSERT INTO `zips` VALUES (2492,'STANDARD',0x00,'Needham',NULL,'Needham Jct','MA','Norfolk County','America/New_York','508,617,774,857,339,781','NA','US',42.28,-71.24,20330);
INSERT INTO `zips` VALUES (2493,'STANDARD',0x00,'Weston',NULL,'Cherry Brook, Hastings, Kendal Green, Silver Hill, Stony Brook','MA','Middlesex County','America/New_York',NULL,'NA','US',42.36,-71.30,10830);
INSERT INTO `zips` VALUES (2494,'STANDARD',0x00,'Needham Heights','Needham, Needham Hgts',NULL,'MA','Norfolk County','America/New_York','339,781,508,617,774,857','NA','US',42.30,-71.23,9810);
INSERT INTO `zips` VALUES (2495,'PO BOX',0x00,'Nonantum','Newton',NULL,'MA','Middlesex County','America/New_York','617','NA','US',42.33,-71.20,30);
INSERT INTO `zips` VALUES (2532,'STANDARD',0x00,'Buzzards Bay','Bourne',NULL,'MA','Barnstable County','America/New_York','508,774','NA','US',41.75,-70.61,9790);
INSERT INTO `zips` VALUES (2534,'PO BOX',0x00,'Cataumet',NULL,NULL,'MA','Barnstable County','America/New_York','508','NA','US',41.66,-70.61,916);
INSERT INTO `zips` VALUES (2535,'STANDARD',0x00,'Chilmark','Aquinnah, Gay Head',NULL,'MA','Dukes County','America/New_York','508','NA','US',41.34,-70.74,1030);
INSERT INTO `zips` VALUES (2536,'STANDARD',0x00,'East Falmouth','E Falmouth, Ea Falmouth, Hatchville, Teaticket, Waquoit',NULL,'MA','Barnstable County','America/New_York','508','NA','US',41.56,-70.55,17840);
INSERT INTO `zips` VALUES (2537,'STANDARD',0x00,'East Sandwich','E Sandwich',NULL,'MA','Barnstable County','America/New_York','508','NA','US',41.73,-70.43,5860);
INSERT INTO `zips` VALUES (2538,'STANDARD',0x00,'East Wareham','E Wareham',NULL,'MA','Plymouth County','America/New_York','774','NA','US',41.78,-70.65,4110);
INSERT INTO `zips` VALUES (2539,'STANDARD',0x00,'Edgartown',NULL,'Chappaquiddick Island','MA','Dukes County','America/New_York','508,774','NA','US',41.38,-70.53,4910);
INSERT INTO `zips` VALUES (2540,'STANDARD',0x00,'Falmouth',NULL,NULL,'MA','Barnstable County','America/New_York','508,774','NA','US',41.56,-70.62,6320);
INSERT INTO `zips` VALUES (2541,'PO BOX',0x00,'Falmouth',NULL,NULL,'MA','Barnstable County','America/New_York','508','NA','US',41.54,-70.60,594);
INSERT INTO `zips` VALUES (2542,'STANDARD',0x00,'Buzzards Bay','Otis Angb','Otis A F B, Otis Air National Guard, Otis Ang','MA','Barnstable County','America/New_York','508,774','NA','US',41.71,-70.55,560);
INSERT INTO `zips` VALUES (2543,'STANDARD',0x00,'Woods Hole','Falmouth','Woodshole','MA','Barnstable County','America/New_York','508,774','NA','US',41.52,-70.66,700);
INSERT INTO `zips` VALUES (2552,'PO BOX',0x00,'Menemsha',NULL,'Chilmark','MA','Dukes County','America/New_York',NULL,'NA','US',41.34,-70.73,46);
INSERT INTO `zips` VALUES (2553,'PO BOX',0x00,'Monument Beach','Monument Bch',NULL,'MA','Barnstable County','America/New_York','508','NA','US',41.71,-70.62,1171);
INSERT INTO `zips` VALUES (2554,'STANDARD',0x00,'Nantucket',NULL,NULL,'MA','Nantucket County','America/New_York','508,774','NA','US',41.27,-70.10,9870);
INSERT INTO `zips` VALUES (2556,'STANDARD',0x00,'North Falmouth','N Falmouth',NULL,'MA','Barnstable County','America/New_York','508,774','NA','US',41.64,-70.63,3210);
INSERT INTO `zips` VALUES (2557,'STANDARD',0x00,'Oak Bluffs',NULL,NULL,'MA','Dukes County','America/New_York',NULL,'NA','US',41.45,-70.56,2660);
INSERT INTO `zips` VALUES (2558,'PO BOX',0x00,'Onset',NULL,NULL,'MA','Plymouth County','America/New_York','508','NA','US',41.74,-70.66,2189);
INSERT INTO `zips` VALUES (2559,'STANDARD',0x00,'Pocasset',NULL,'Bourne','MA','Barnstable County','America/New_York','508,774','NA','US',41.69,-70.63,2920);
INSERT INTO `zips` VALUES (2561,'PO BOX',0x00,'Sagamore',NULL,NULL,'MA','Barnstable County','America/New_York','508','NA','US',41.77,-70.53,884);
INSERT INTO `zips` VALUES (2562,'STANDARD',0x00,'Sagamore Beach','Sagamore Bch',NULL,'MA','Barnstable County','America/New_York','508,774','NA','US',41.79,-70.53,3050);
INSERT INTO `zips` VALUES (2563,'STANDARD',0x00,'Sandwich',NULL,NULL,'MA','Barnstable County','America/New_York','774,508','NA','US',41.75,-70.49,9860);
INSERT INTO `zips` VALUES (2564,'PO BOX',0x00,'Siasconset','Nantucket','Sconset','MA','Nantucket County','America/New_York','508','NA','US',41.27,-70.00,423);
INSERT INTO `zips` VALUES (2565,'PO BOX',0x00,'Silver Beach','N Falmouth, North Falmouth',NULL,'MA','Barnstable County','America/New_York','508','NA','US',41.63,-70.64,0);
INSERT INTO `zips` VALUES (2568,'STANDARD',0x00,'Vineyard Haven','Vineyard Hvn','North Tisbury, Tisbury, West Tisbury','MA','Dukes County','America/New_York','508,774','NA','US',41.45,-70.60,6830);
INSERT INTO `zips` VALUES (2571,'STANDARD',0x00,'Wareham',NULL,NULL,'MA','Plymouth County','America/New_York','508,774','NA','US',41.76,-70.71,9080);
INSERT INTO `zips` VALUES (2573,'PO BOX',0x00,'West Chop','Vineyard Haven, Vineyard Hvn','Tisbury','MA','Dukes County','America/New_York',NULL,'NA','US',41.45,-70.60,0);
INSERT INTO `zips` VALUES (2574,'PO BOX',0x00,'West Falmouth','W Falmouth',NULL,'MA','Barnstable County','America/New_York','508','NA','US',41.60,-70.64,1104);
INSERT INTO `zips` VALUES (2575,'PO BOX',0x00,'West Tisbury',NULL,'Tisbury','MA','Dukes County','America/New_York',NULL,'NA','US',41.38,-70.67,1679);
INSERT INTO `zips` VALUES (2576,'STANDARD',0x00,'West Wareham',NULL,'W Wareham','MA','Plymouth County','America/New_York','508,774','NA','US',41.78,-70.75,3720);
INSERT INTO `zips` VALUES (2584,'PO BOX',0x00,'Nantucket',NULL,NULL,'MA','Nantucket County','America/New_York','508','NA','US',41.26,-70.01,1904);
INSERT INTO `zips` VALUES (2601,'STANDARD',0x00,'Hyannis',NULL,NULL,'MA','Barnstable County','America/New_York','508,774','NA','US',41.65,-70.29,12890);
INSERT INTO `zips` VALUES (2630,'STANDARD',0x00,'Barnstable',NULL,NULL,'MA','Barnstable County','America/New_York','508,774','NA','US',41.70,-70.30,2000);
INSERT INTO `zips` VALUES (2631,'STANDARD',0x00,'Brewster',NULL,NULL,'MA','Barnstable County','America/New_York','508,774','NA','US',41.76,-70.08,8660);
INSERT INTO `zips` VALUES (2632,'STANDARD',0x00,'Centerville',NULL,NULL,'MA','Barnstable County','America/New_York','508,774','NA','US',41.66,-70.34,9890);
INSERT INTO `zips` VALUES (2633,'STANDARD',0x00,'Chatham',NULL,NULL,'MA','Barnstable County','America/New_York','508,774','NA','US',41.67,-69.96,3550);
INSERT INTO `zips` VALUES (2634,'PO BOX',0x00,'Centerville',NULL,'Barnstable','MA','Barnstable County','America/New_York','508','NA','US',41.66,-70.34,19);
INSERT INTO `zips` VALUES (2635,'STANDARD',0x00,'Cotuit',NULL,NULL,'MA','Barnstable County','America/New_York','508','NA','US',41.62,-70.44,3340);
INSERT INTO `zips` VALUES (2636,'STANDARD',0x01,'Centerville',NULL,NULL,'MA','Barnstable County','America/New_York','508','NA','US',41.64,-70.34,0);
INSERT INTO `zips` VALUES (2637,'PO BOX',0x00,'Cummaquid',NULL,NULL,'MA','Barnstable County','America/New_York','508','NA','US',41.71,-70.27,564);
INSERT INTO `zips` VALUES (2638,'STANDARD',0x00,'Dennis',NULL,NULL,'MA','Barnstable County','America/New_York','508,774','NA','US',41.73,-70.20,2720);
INSERT INTO `zips` VALUES (2639,'STANDARD',0x00,'Dennis Port','Dennisport',NULL,'MA','Barnstable County','America/New_York','508','NA','US',41.66,-70.13,2740);
INSERT INTO `zips` VALUES (2641,'PO BOX',0x00,'East Dennis',NULL,'E Dennis','MA','Barnstable County','America/New_York','508','NA','US',41.75,-70.15,1542);
INSERT INTO `zips` VALUES (2642,'STANDARD',0x00,'Eastham',NULL,NULL,'MA','Barnstable County','America/New_York','508','NA','US',41.83,-69.96,3470);
INSERT INTO `zips` VALUES (2643,'PO BOX',0x00,'East Orleans',NULL,NULL,'MA','Barnstable County','America/New_York','508','NA','US',41.80,-69.94,1113);
INSERT INTO `zips` VALUES (2644,'STANDARD',0x00,'Forestdale',NULL,'Sandwich','MA','Barnstable County','America/New_York','508','NA','US',41.68,-70.50,4150);
INSERT INTO `zips` VALUES (2645,'STANDARD',0x00,'Harwich','E Harwich, East Harwich','Hardwich','MA','Barnstable County','America/New_York','508,774','NA','US',41.69,-70.07,9240);
INSERT INTO `zips` VALUES (2646,'STANDARD',0x00,'Harwich Port',NULL,'Harwichport','MA','Barnstable County','America/New_York','508','NA','US',41.67,-70.07,1820);
INSERT INTO `zips` VALUES (2647,'PO BOX',0x00,'Hyannis Port',NULL,NULL,'MA','Barnstable County','America/New_York','508','NA','US',41.63,-70.31,393);
INSERT INTO `zips` VALUES (2648,'STANDARD',0x00,'Marstons Mills','Marstons Mls',NULL,'MA','Barnstable County','America/New_York','508','NA','US',41.67,-70.40,7110);
INSERT INTO `zips` VALUES (2649,'STANDARD',0x00,'Mashpee',NULL,'New Seabury, South Mashpee','MA','Barnstable County','America/New_York','508','NA','US',41.65,-70.48,13490);
INSERT INTO `zips` VALUES (2650,'STANDARD',0x00,'North Chatham',NULL,'N Chatham','MA','Barnstable County','America/New_York','508','NA','US',41.70,-69.95,790);
INSERT INTO `zips` VALUES (2651,'PO BOX',0x00,'North Eastham',NULL,NULL,'MA','Barnstable County','America/New_York','508','NA','US',41.87,-70.00,1610);
INSERT INTO `zips` VALUES (2652,'PO BOX',0x00,'North Truro',NULL,NULL,'MA','Barnstable County','America/New_York','508','NA','US',42.04,-70.09,766);
INSERT INTO `zips` VALUES (2653,'STANDARD',0x00,'Orleans',NULL,NULL,'MA','Barnstable County','America/New_York','508,774','NA','US',41.79,-70.00,4640);
INSERT INTO `zips` VALUES (2655,'STANDARD',0x00,'Osterville',NULL,NULL,'MA','Barnstable County','America/New_York','508,774','NA','US',41.62,-70.38,3320);
INSERT INTO `zips` VALUES (2657,'STANDARD',0x00,'Provincetown',NULL,NULL,'MA','Barnstable County','America/New_York','508,774','NA','US',42.06,-70.20,3410);
INSERT INTO `zips` VALUES (2659,'STANDARD',0x00,'South Chatham',NULL,NULL,'MA','Barnstable County','America/New_York','508','NA','US',41.68,-70.02,1060);
INSERT INTO `zips` VALUES (2660,'STANDARD',0x00,'South Dennis',NULL,'S Dennis','MA','Barnstable County','America/New_York','508','NA','US',41.70,-70.15,5380);
INSERT INTO `zips` VALUES (2661,'PO BOX',0x00,'South Harwich',NULL,NULL,'MA','Barnstable County','America/New_York','508','NA','US',41.67,-70.04,365);
INSERT INTO `zips` VALUES (2662,'PO BOX',0x00,'South Orleans',NULL,NULL,'MA','Barnstable County','America/New_York','508','NA','US',41.75,-69.99,862);
INSERT INTO `zips` VALUES (2663,'PO BOX',0x00,'South Wellfleet','S Wellfleet',NULL,'MA','Barnstable County','America/New_York','508','NA','US',41.89,-70.01,611);
INSERT INTO `zips` VALUES (2664,'STANDARD',0x00,'South Yarmouth','Bass River, S Yarmouth','So Yarmouth, Yarmouth','MA','Barnstable County','America/New_York','508','NA','US',41.67,-70.20,8460);
INSERT INTO `zips` VALUES (2666,'PO BOX',0x00,'Truro',NULL,NULL,'MA','Barnstable County','America/New_York','508','NA','US',42.00,-70.06,1057);
INSERT INTO `zips` VALUES (2667,'STANDARD',0x00,'Wellfleet',NULL,NULL,'MA','Barnstable County','America/New_York','508,774','NA','US',41.93,-70.03,2340);
INSERT INTO `zips` VALUES (2668,'STANDARD',0x00,'West Barnstable','W Barnstble','W Barnstable','MA','Barnstable County','America/New_York','508','NA','US',41.70,-70.37,3140);
INSERT INTO `zips` VALUES (2669,'PO BOX',0x00,'West Chatham',NULL,NULL,'MA','Barnstable County','America/New_York','508','NA','US',41.68,-69.99,812);
INSERT INTO `zips` VALUES (2670,'STANDARD',0x00,'West Dennis',NULL,'W Dennis','MA','Barnstable County','America/New_York','508','NA','US',41.66,-70.16,1280);
INSERT INTO `zips` VALUES (2671,'STANDARD',0x00,'West Harwich',NULL,NULL,'MA','Barnstable County','America/New_York','508','NA','US',41.67,-70.11,1040);
INSERT INTO `zips` VALUES (2672,'PO BOX',0x00,'West Hyannisport','W Hyannisprt',NULL,'MA','Barnstable County','America/New_York','508','NA','US',41.64,-70.31,577);
INSERT INTO `zips` VALUES (2673,'STANDARD',0x00,'West Yarmouth','W Yarmouth','South Yarmouth, Yarmouth','MA','Barnstable County','America/New_York','508,774','NA','US',41.65,-70.24,7530);
INSERT INTO `zips` VALUES (2675,'STANDARD',0x00,'Yarmouth Port',NULL,'Yarmouth, Yarmouthport','MA','Barnstable County','America/New_York','508','NA','US',41.70,-70.22,6030);
INSERT INTO `zips` VALUES (2702,'STANDARD',0x00,'Assonet',NULL,'Freetown','MA','Bristol County','America/New_York','508,774','NA','US',41.78,-71.06,4050);
INSERT INTO `zips` VALUES (2703,'STANDARD',0x00,'Attleboro','S Attleboro, South Attleboro',NULL,'MA','Bristol County','America/New_York','508,774','NA','US',41.93,-71.29,40590);
INSERT INTO `zips` VALUES (2712,'PO BOX',0x00,'Chartley',NULL,'Norton','MA','Bristol County','America/New_York','508','NA','US',41.97,-71.18,272);
INSERT INTO `zips` VALUES (2713,'PO BOX',0x00,'Cuttyhunk',NULL,'Gosnold','MA','Dukes County','America/New_York',NULL,'NA','US',41.44,-70.90,32);
INSERT INTO `zips` VALUES (2714,'PO BOX',0x00,'Dartmouth',NULL,NULL,'MA','Bristol County','America/New_York','508','NA','US',41.56,-71.00,47);
INSERT INTO `zips` VALUES (2715,'STANDARD',0x00,'Dighton',NULL,NULL,'MA','Bristol County','America/New_York','508,774','NA','US',41.82,-71.16,3290);
INSERT INTO `zips` VALUES (2717,'STANDARD',0x00,'East Freetown',NULL,NULL,'MA','Bristol County','America/New_York','508','NA','US',41.75,-70.97,4710);
INSERT INTO `zips` VALUES (2718,'STANDARD',0x00,'East Taunton',NULL,'Taunton','MA','Bristol County','America/New_York','508,774','NA','US',41.87,-71.01,6500);
INSERT INTO `zips` VALUES (2719,'STANDARD',0x00,'Fairhaven',NULL,NULL,'MA','Bristol County','America/New_York','508','NA','US',41.63,-70.90,13840);
INSERT INTO `zips` VALUES (2720,'STANDARD',0x00,'Fall River',NULL,NULL,'MA','Bristol County','America/New_York','508,774','NA','US',41.73,-71.12,24080);
INSERT INTO `zips` VALUES (2721,'STANDARD',0x00,'Fall River',NULL,NULL,'MA','Bristol County','America/New_York','508,774','NA','US',41.68,-71.15,20150);
INSERT INTO `zips` VALUES (2722,'PO BOX',0x00,'Fall River',NULL,NULL,'MA','Bristol County','America/New_York','508','NA','US',41.71,-71.10,738);
INSERT INTO `zips` VALUES (2723,'STANDARD',0x00,'Fall River',NULL,NULL,'MA','Bristol County','America/New_York','508,774','NA','US',41.69,-71.13,11530);
INSERT INTO `zips` VALUES (2724,'STANDARD',0x00,'Fall River',NULL,NULL,'MA','Bristol County','America/New_York','508','NA','US',41.68,-71.18,13000);
INSERT INTO `zips` VALUES (2725,'STANDARD',0x00,'Somerset',NULL,'Fall River','MA','Bristol County','America/New_York','508','NA','US',41.72,-71.19,2330);
INSERT INTO `zips` VALUES (2726,'STANDARD',0x00,'Somerset',NULL,'Fall River','MA','Bristol County','America/New_York','508','NA','US',41.73,-71.15,14380);
INSERT INTO `zips` VALUES (2738,'STANDARD',0x00,'Marion',NULL,NULL,'MA','Plymouth County','America/New_York','508,774','NA','US',41.70,-70.76,4950);
INSERT INTO `zips` VALUES (2739,'STANDARD',0x00,'Mattapoisett',NULL,NULL,'MA','Plymouth County','America/New_York','508,774','NA','US',41.66,-70.80,6220);
INSERT INTO `zips` VALUES (2740,'STANDARD',0x00,'New Bedford',NULL,NULL,'MA','Bristol County','America/New_York','508,774','NA','US',41.64,-70.94,33170);
INSERT INTO `zips` VALUES (2741,'PO BOX',0x00,'New Bedford',NULL,NULL,'MA','Bristol County','America/New_York','508','NA','US',41.66,-70.93,157);
INSERT INTO `zips` VALUES (2742,'PO BOX',0x00,'New Bedford',NULL,NULL,'MA','Bristol County','America/New_York','508','NA','US',41.66,-70.93,334);
INSERT INTO `zips` VALUES (2743,'STANDARD',0x00,'Acushnet','New Bedford',NULL,'MA','Bristol County','America/New_York','508','NA','US',41.68,-70.90,9580);
INSERT INTO `zips` VALUES (2744,'STANDARD',0x00,'New Bedford',NULL,NULL,'MA','Bristol County','America/New_York','508,774','NA','US',41.61,-70.91,9450);
INSERT INTO `zips` VALUES (2745,'STANDARD',0x00,'New Bedford','Acushnet',NULL,'MA','Bristol County','America/New_York','508','NA','US',41.70,-70.95,21310);
INSERT INTO `zips` VALUES (2746,'STANDARD',0x00,'New Bedford',NULL,NULL,'MA','Bristol County','America/New_York','508,774','NA','US',41.66,-70.93,11960);
INSERT INTO `zips` VALUES (2747,'STANDARD',0x00,'North Dartmouth','Dartmouth, N Dartmouth',NULL,'MA','Bristol County','America/New_York','508','NA','US',41.64,-71.00,16910);
INSERT INTO `zips` VALUES (2748,'STANDARD',0x00,'South Dartmouth','Dartmouth, Nonquitt, S Dartmouth','Padanaram The Packet','MA','Bristol County','America/New_York','508','NA','US',41.55,-70.98,10470);
INSERT INTO `zips` VALUES (2760,'STANDARD',0x00,'North Attleboro','N Attleboro','No Attleboro','MA','Bristol County','America/New_York','508,774','NA','US',41.97,-71.33,26000);
INSERT INTO `zips` VALUES (2761,'PO BOX',0x00,'North Attleboro','N Attleboro',NULL,'MA','Bristol County','America/New_York','508','NA','US',41.97,-71.33,356);
INSERT INTO `zips` VALUES (2762,'STANDARD',0x00,'Plainville',NULL,'N Attleboro, North Attleboro','MA','Norfolk County','America/New_York',NULL,'NA','US',42.00,-71.33,8470);
INSERT INTO `zips` VALUES (2763,'STANDARD',0x00,'Attleboro Falls','Attleboro Fls, N Attleboro, North Attleboro',NULL,'MA','Bristol County','America/New_York','508,774','NA','US',41.97,-71.31,2030);
INSERT INTO `zips` VALUES (2764,'STANDARD',0x00,'North Dighton','N Dighton',NULL,'MA','Bristol County','America/New_York','508','NA','US',41.85,-71.15,3980);
INSERT INTO `zips` VALUES (2766,'STANDARD',0x00,'Norton',NULL,NULL,'MA','Bristol County','America/New_York','508,774','NA','US',41.96,-71.18,16530);
INSERT INTO `zips` VALUES (2767,'STANDARD',0x00,'Raynham',NULL,NULL,'MA','Bristol County','America/New_York','508','NA','US',41.93,-71.04,13220);
INSERT INTO `zips` VALUES (2768,'PO BOX',0x00,'Raynham Center','Raynham Ctr','Raynham','MA','Bristol County','America/New_York','508','NA','US',41.93,-71.04,502);
INSERT INTO `zips` VALUES (2769,'STANDARD',0x00,'Rehoboth',NULL,NULL,'MA','Bristol County','America/New_York','508,774','NA','US',41.83,-71.26,11520);
INSERT INTO `zips` VALUES (2770,'STANDARD',0x00,'Rochester',NULL,NULL,'MA','Plymouth County','America/New_York','508,774','NA','US',41.73,-70.81,5320);
INSERT INTO `zips` VALUES (2771,'STANDARD',0x00,'Seekonk',NULL,NULL,'MA','Bristol County','America/New_York','508,774','NA','US',41.81,-71.33,13890);
INSERT INTO `zips` VALUES (2777,'STANDARD',0x00,'Swansea',NULL,NULL,'MA','Bristol County','America/New_York','508','NA','US',41.75,-71.18,15040);
INSERT INTO `zips` VALUES (2779,'STANDARD',0x00,'Berkley',NULL,NULL,'MA','Bristol County','America/New_York','508','NA','US',41.85,-71.08,6360);
INSERT INTO `zips` VALUES (2780,'STANDARD',0x00,'Taunton',NULL,NULL,'MA','Bristol County','America/New_York','508,774','NA','US',41.90,-71.09,43620);
INSERT INTO `zips` VALUES (2783,'UNIQUE',0x00,'Taunton',NULL,'Chadwicks','MA','Bristol County','America/New_York','508','NA','US',41.90,-71.09,0);
INSERT INTO `zips` VALUES (2790,'STANDARD',0x00,'Westport',NULL,'Horseneck Beach','MA','Bristol County','America/New_York','508,774','NA','US',41.66,-71.10,14990);
INSERT INTO `zips` VALUES (2791,'PO BOX',0x00,'Westport Point','Westport Pt',NULL,'MA','Bristol County','America/New_York','508','NA','US',41.52,-71.07,388);
INSERT INTO `zips` VALUES (2801,'PO BOX',0x00,'Adamsville',NULL,'Little Compton','RI','Newport County','America/New_York','401','NA','US',41.51,-71.16,299);
INSERT INTO `zips` VALUES (2802,'PO BOX',0x00,'Albion',NULL,'Lincoln','RI','Providence County','America/New_York','401','NA','US',41.95,-71.46,770);
INSERT INTO `zips` VALUES (2804,'STANDARD',0x00,'Ashaway',NULL,'Hopkinton','RI','Washington County','America/New_York','401','NA','US',41.42,-71.78,2640);
INSERT INTO `zips` VALUES (2806,'STANDARD',0x00,'Barrington',NULL,NULL,'RI','Bristol County','America/New_York','401','NA','US',41.73,-71.31,16810);
INSERT INTO `zips` VALUES (2807,'PO BOX',0x00,'Block Island','New Shoreham',NULL,'RI','Washington County','America/New_York','401','NA','US',41.16,-71.58,1213);
INSERT INTO `zips` VALUES (2808,'STANDARD',0x00,'Bradford',NULL,'Hopkinton, Westerly','RI','Washington County','America/New_York','401','NA','US',41.39,-71.75,2140);
INSERT INTO `zips` VALUES (2809,'STANDARD',0x00,'Bristol',NULL,NULL,'RI','Bristol County','America/New_York','401','NA','US',41.67,-71.27,17300);
INSERT INTO `zips` VALUES (2812,'STANDARD',0x00,'Carolina','Richmond',NULL,'RI','Washington County','America/New_York','401','NA','US',41.47,-71.64,1310);
INSERT INTO `zips` VALUES (2813,'STANDARD',0x00,'Charlestown',NULL,NULL,'RI','Washington County','America/New_York','401','NA','US',41.38,-71.65,6920);
INSERT INTO `zips` VALUES (2814,'STANDARD',0x00,'Chepachet',NULL,'Glocester','RI','Providence County','America/New_York','401','NA','US',41.91,-71.70,6900);
INSERT INTO `zips` VALUES (2815,'STANDARD',0x00,'Clayville',NULL,'Scituate','RI','Providence County','America/New_York','401','NA','US',41.77,-71.66,240);
INSERT INTO `zips` VALUES (2816,'STANDARD',0x00,'Coventry',NULL,NULL,'RI','Kent County','America/New_York','401','NA','US',41.68,-71.66,29700);
INSERT INTO `zips` VALUES (2817,'STANDARD',0x00,'West Greenwich','W Greenwich',NULL,'RI','Kent County','America/New_York','401','NA','US',41.63,-71.65,5860);
INSERT INTO `zips` VALUES (2818,'STANDARD',0x00,'East Greenwich','E Greenwich','Warwick','RI','Kent County','America/New_York','401','NA','US',41.63,-71.50,18030);
INSERT INTO `zips` VALUES (2822,'STANDARD',0x00,'Exeter','Escoheag',NULL,'RI','Washington County','America/New_York','401','NA','US',41.56,-71.68,5550);
INSERT INTO `zips` VALUES (2823,'PO BOX',0x00,'Fiskeville',NULL,'Cranston','RI','Providence County','America/New_York','401','NA','US',41.73,-71.54,302);
INSERT INTO `zips` VALUES (2824,'PO BOX',0x00,'Forestdale',NULL,NULL,'RI','Providence County','America/New_York','401','NA','US',41.97,-71.55,394);
INSERT INTO `zips` VALUES (2825,'STANDARD',0x00,'Foster',NULL,'Scituate','RI','Providence County','America/New_York','401','NA','US',41.85,-71.76,4960);
INSERT INTO `zips` VALUES (2826,'PO BOX',0x00,'Glendale',NULL,'Burrillville','RI','Providence County','America/New_York','401','NA','US',41.98,-71.65,880);
INSERT INTO `zips` VALUES (2827,'STANDARD',0x00,'Greene','Coventry',NULL,'RI','Kent County','America/New_York','401','NA','US',41.69,-71.74,2130);
INSERT INTO `zips` VALUES (2828,'STANDARD',0x00,'Greenville',NULL,'Smithfield','RI','Providence County','America/New_York','401','NA','US',41.87,-71.55,6660);
INSERT INTO `zips` VALUES (2829,'PO BOX',0x00,'Harmony',NULL,'Glocester','RI','Providence County','America/New_York','401','NA','US',41.88,-71.61,405);
INSERT INTO `zips` VALUES (2830,'STANDARD',0x00,'Harrisville','Burrillville',NULL,'RI','Providence County','America/New_York','401','NA','US',41.96,-71.67,5940);
INSERT INTO `zips` VALUES (2831,'STANDARD',0x00,'Hope',NULL,'Scituate','RI','Providence County','America/New_York','401','NA','US',41.75,-71.56,3860);
INSERT INTO `zips` VALUES (2832,'STANDARD',0x00,'Hope Valley','Richmond','Hopkinton','RI','Washington County','America/New_York','401','NA','US',41.51,-71.72,4410);
INSERT INTO `zips` VALUES (2833,'STANDARD',0x00,'Hopkinton',NULL,NULL,'RI','Washington County','America/New_York','401','NA','US',41.48,-71.77,550);
INSERT INTO `zips` VALUES (2835,'STANDARD',0x00,'Jamestown',NULL,NULL,'RI','Newport County','America/New_York','401','NA','US',41.48,-71.36,5230);
INSERT INTO `zips` VALUES (2836,'STANDARD',0x00,'Kenyon','Richmond','Charlestown','RI','Washington County','America/New_York','401','NA','US',41.45,-71.62,91);
INSERT INTO `zips` VALUES (2837,'STANDARD',0x00,'Little Compton','L Compton','Adamsville','RI','Newport County','America/New_York','401','NA','US',41.50,-71.16,3060);
INSERT INTO `zips` VALUES (2838,'STANDARD',0x00,'Manville',NULL,'Lincoln','RI','Providence County','America/New_York','401','NA','US',41.96,-71.47,3020);
INSERT INTO `zips` VALUES (2839,'STANDARD',0x00,'Mapleville',NULL,'Burrillville','RI','Providence County','America/New_York','401','NA','US',41.94,-71.64,1620);
INSERT INTO `zips` VALUES (2840,'STANDARD',0x00,'Newport',NULL,'Middletown','RI','Newport County','America/New_York','401','NA','US',41.47,-71.30,18320);
INSERT INTO `zips` VALUES (2841,'STANDARD',0x00,'Newport',NULL,'Netc','RI','Newport County','America/New_York','401','NA','US',41.51,-71.33,353);
INSERT INTO `zips` VALUES (2842,'STANDARD',0x00,'Middletown',NULL,NULL,'RI','Newport County','America/New_York','401','NA','US',41.51,-71.27,15150);
INSERT INTO `zips` VALUES (2852,'STANDARD',0x00,'North Kingstown','N Kingstown','Davisville, Wickford','RI','Washington County','America/New_York','401','NA','US',41.55,-71.46,21530);
INSERT INTO `zips` VALUES (2854,'STANDARD',0x01,'North Kingstown','N Kingstown',NULL,'RI','Washington County','America/New_York','401','NA','US',41.59,-71.45,0);
INSERT INTO `zips` VALUES (2857,'STANDARD',0x00,'North Scituate','N Scituate, Scituate','Glocester','RI','Providence County','America/New_York','401','NA','US',41.83,-71.63,8010);
INSERT INTO `zips` VALUES (2858,'STANDARD',0x00,'Oakland',NULL,'Burrillville','RI','Providence County','America/New_York','401','NA','US',41.97,-71.65,520);
INSERT INTO `zips` VALUES (2859,'STANDARD',0x00,'Pascoag',NULL,'Glocester','RI','Providence County','America/New_York','401','NA','US',41.95,-71.70,5610);
INSERT INTO `zips` VALUES (2860,'STANDARD',0x00,'Pawtucket',NULL,NULL,'RI','Providence County','America/New_York','401','NA','US',41.87,-71.37,38130);
INSERT INTO `zips` VALUES (2861,'STANDARD',0x00,'Pawtucket',NULL,'Darlington','RI','Providence County','America/New_York','401','NA','US',41.88,-71.35,23950);
INSERT INTO `zips` VALUES (2862,'PO BOX',0x00,'Pawtucket',NULL,NULL,'RI','Providence County','America/New_York','401','NA','US',41.87,-71.37,912);
INSERT INTO `zips` VALUES (2863,'STANDARD',0x00,'Central Falls',NULL,NULL,'RI','Providence County','America/New_York','401','NA','US',41.89,-71.39,15900);
INSERT INTO `zips` VALUES (2864,'STANDARD',0x00,'Cumberland',NULL,NULL,'RI','Providence County','America/New_York','401','NA','US',41.94,-71.41,32390);
INSERT INTO `zips` VALUES (2865,'STANDARD',0x00,'Lincoln',NULL,NULL,'RI','Providence County','America/New_York','401','NA','US',41.91,-71.45,16580);
INSERT INTO `zips` VALUES (2871,'STANDARD',0x00,'Portsmouth',NULL,NULL,'RI','Newport County','America/New_York','401','NA','US',41.60,-71.25,16410);
INSERT INTO `zips` VALUES (2872,'PO BOX',0x00,'Prudence Island','Prudence Isl','Portsmouth','RI','Newport County','America/New_York','401','NA','US',41.60,-71.31,160);
INSERT INTO `zips` VALUES (2873,'PO BOX',0x00,'Rockville',NULL,'Hopkinton','RI','Washington County','America/New_York','401','NA','US',41.53,-71.78,301);
INSERT INTO `zips` VALUES (2874,'STANDARD',0x00,'Saunderstown',NULL,'Narragansett, North Kingstown','RI','Washington County','America/New_York','401','NA','US',41.51,-71.44,5810);
INSERT INTO `zips` VALUES (2875,'STANDARD',0x00,'Shannock','Richmond',NULL,'RI','Washington County','America/New_York','401','NA','US',41.46,-71.64,350);
INSERT INTO `zips` VALUES (2876,'STANDARD',0x00,'Slatersville',NULL,NULL,'RI','Providence County','America/New_York','401','NA','US',41.99,-71.59,1180);
INSERT INTO `zips` VALUES (2877,'STANDARD',0x00,'Slocum',NULL,NULL,'RI','Washington County','America/New_York','401','NA','US',41.52,-71.54,106);
INSERT INTO `zips` VALUES (2878,'STANDARD',0x00,'Tiverton',NULL,NULL,'RI','Newport County','America/New_York','401','NA','US',41.65,-71.20,14500);
INSERT INTO `zips` VALUES (2879,'STANDARD',0x00,'Wakefield','Narragansett, Peace Dale, S Kingstown, South Kingstown','East Matunuck, Green Hill, Jerusalem, Matunuck','RI','Washington County','America/New_York','401','NA','US',41.45,-71.51,18750);
INSERT INTO `zips` VALUES (2880,'PO BOX',0x00,'Wakefield',NULL,NULL,'RI','Washington County','America/New_York','401','NA','US',41.45,-71.51,684);
INSERT INTO `zips` VALUES (2881,'STANDARD',0x00,'Kingston',NULL,'Wakefield','RI','Washington County','America/New_York','401','NA','US',41.48,-71.52,1940);
INSERT INTO `zips` VALUES (2882,'STANDARD',0x00,'Narragansett','Point Judith','Bonnet Shores, Galilee','RI','Washington County','America/New_York','401','NA','US',41.39,-71.48,10070);
INSERT INTO `zips` VALUES (2883,'PO BOX',0x00,'Peace Dale','S Kingstown, South Kingstown','Wakefield','RI','Washington County','America/New_York','401','NA','US',41.45,-71.50,217);
INSERT INTO `zips` VALUES (2885,'STANDARD',0x00,'Warren',NULL,NULL,'RI','Bristol County','America/New_York','401','NA','US',41.72,-71.26,9180);
INSERT INTO `zips` VALUES (2886,'STANDARD',0x00,'Warwick',NULL,NULL,'RI','Kent County','America/New_York','401','NA','US',41.70,-71.46,26080);
INSERT INTO `zips` VALUES (2887,'PO BOX',0x00,'Warwick',NULL,NULL,'RI','Kent County','America/New_York','401','NA','US',41.70,-71.42,344);
INSERT INTO `zips` VALUES (2888,'STANDARD',0x00,'Warwick',NULL,NULL,'RI','Kent County','America/New_York','401','NA','US',41.75,-71.41,18150);
INSERT INTO `zips` VALUES (2889,'STANDARD',0x00,'Warwick',NULL,'Conimicut','RI','Kent County','America/New_York','401','NA','US',41.70,-71.42,25560);
INSERT INTO `zips` VALUES (2891,'STANDARD',0x00,'Westerly',NULL,'Misquamicut, Watch Hill','RI','Washington County','America/New_York','401','NA','US',41.37,-71.81,19460);
INSERT INTO `zips` VALUES (2892,'STANDARD',0x00,'West Kingston','Richmond','South Kingstown','RI','Washington County','America/New_York','401','NA','US',41.50,-71.59,5000);
INSERT INTO `zips` VALUES (2893,'STANDARD',0x00,'West Warwick',NULL,'W Warwick','RI','Kent County','America/New_York','401','NA','US',41.69,-71.51,25880);
INSERT INTO `zips` VALUES (2894,'STANDARD',0x00,'Wood River Junction','Wood River Jt',NULL,'RI','Washington County','America/New_York','401','NA','US',41.45,-71.70,760);
INSERT INTO `zips` VALUES (2895,'STANDARD',0x00,'Woonsocket',NULL,NULL,'RI','Providence County','America/New_York','401','NA','US',41.99,-71.50,33800);
INSERT INTO `zips` VALUES (2896,'STANDARD',0x00,'North Smithfield','N Smithfield',NULL,'RI','Providence County','America/New_York','401','NA','US',41.95,-71.55,9360);
INSERT INTO `zips` VALUES (2898,'STANDARD',0x00,'Wyoming','Richmond',NULL,'RI','Washington County','America/New_York','401','NA','US',41.52,-71.67,1860);
INSERT INTO `zips` VALUES (2901,'PO BOX',0x00,'Providence',NULL,NULL,'RI','Providence County','America/New_York','401','NA','US',41.82,-71.41,299);
INSERT INTO `zips` VALUES (2902,'UNIQUE',0x00,'Providence',NULL,'Providence Journal','RI','Providence County','America/New_York','401','NA','US',41.82,-71.41,69);
INSERT INTO `zips` VALUES (2903,'STANDARD',0x00,'Providence',NULL,NULL,'RI','Providence County','America/New_York','401','NA','US',41.82,-71.41,5350);
INSERT INTO `zips` VALUES (2904,'STANDARD',0x00,'Providence','N Providence, North Providence','No Providence','RI','Providence County','America/New_York','401','NA','US',41.86,-71.44,24910);
INSERT INTO `zips` VALUES (2905,'STANDARD',0x00,'Providence','Cranston',NULL,'RI','Providence County','America/New_York','401','NA','US',41.78,-71.40,21040);
INSERT INTO `zips` VALUES (2906,'STANDARD',0x00,'Providence',NULL,NULL,'RI','Providence County','America/New_York','401','NA','US',41.84,-71.39,19250);
INSERT INTO `zips` VALUES (2907,'STANDARD',0x00,'Providence','Cranston',NULL,'RI','Providence County','America/New_York','401','NA','US',41.80,-71.42,25280);
INSERT INTO `zips` VALUES (2908,'STANDARD',0x00,'Providence','N Providence, North Providence',NULL,'RI','Providence County','America/New_York','401','NA','US',41.84,-71.44,29520);
INSERT INTO `zips` VALUES (2909,'STANDARD',0x00,'Providence',NULL,NULL,'RI','Providence County','America/New_York','401','NA','US',41.82,-71.45,35900);
INSERT INTO `zips` VALUES (2910,'STANDARD',0x00,'Cranston','Providence',NULL,'RI','Providence County','America/New_York','401','NA','US',41.77,-71.44,20320);
INSERT INTO `zips` VALUES (2911,'STANDARD',0x00,'North Providence','N Providence, Providence','Centerdale, Centredale, Centredale Finance Branch, No Providence','RI','Providence County','America/New_York','401','NA','US',41.85,-71.47,13450);
INSERT INTO `zips` VALUES (2912,'UNIQUE',0x00,'Providence','Brown Station','Brown University','RI','Providence County','America/New_York','401','NA','US',41.83,-71.40,375);
INSERT INTO `zips` VALUES (2914,'STANDARD',0x00,'East Providence','E Providence',NULL,'RI','Providence County','America/New_York','401','NA','US',41.81,-71.37,18220);
INSERT INTO `zips` VALUES (2915,'STANDARD',0x00,'Riverside',NULL,'East Providence','RI','Providence County','America/New_York','401','NA','US',41.77,-71.35,14630);
INSERT INTO `zips` VALUES (2916,'STANDARD',0x00,'Rumford',NULL,'East Providence','RI','Providence County','America/New_York','401','NA','US',41.84,-71.35,7730);
INSERT INTO `zips` VALUES (2917,'STANDARD',0x00,'Smithfield',NULL,'Esmond','RI','Providence County','America/New_York','401','NA','US',41.90,-71.53,10610);
INSERT INTO `zips` VALUES (2918,'UNIQUE',0x00,'Providence','Friar Station','Providence College','RI','Providence County','America/New_York','401','NA','US',41.82,-71.41,36);
INSERT INTO `zips` VALUES (2919,'STANDARD',0x00,'Johnston','Providence',NULL,'RI','Providence County','America/New_York','401','NA','US',41.83,-71.52,25900);
INSERT INTO `zips` VALUES (2920,'STANDARD',0x00,'Cranston',NULL,NULL,'RI','Providence County','America/New_York','401','NA','US',41.77,-71.47,31070);
INSERT INTO `zips` VALUES (2921,'STANDARD',0x00,'Cranston',NULL,NULL,'RI','Providence County','America/New_York','401','NA','US',41.76,-71.48,11690);
INSERT INTO `zips` VALUES (2940,'PO BOX',0x00,'Providence',NULL,NULL,'RI','Providence County','America/New_York','401','NA','US',41.82,-71.41,1143);
/*!40000 ALTER TABLE `zips` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

