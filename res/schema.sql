CREATE TABLE "platform"  (
	`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
	`name`	TEXT NOT NULL,
	`command`	TEXT,
    `icon`  TEXT,
    `background` TEXT,
    `type` TEXT DEFAULT 'complete'
);
CREATE TABLE "mehstation" (
    `name` TEXT NOT NULL PRIMARY KEY UNIQUE,
    `value` TEXT
);
CREATE TABLE "executable_resource" (
	`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,
	`executable_id`	INTEGER NOT NULL,
	`type`	TEXT DEFAULT '',
	`filepath`	TEXT DEFAULT ''
);
CREATE TABLE "executable" (
	`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,
	`display_name`	TEXT DEFAULT '',
	`filepath`	TEXT DEFAULT '',
	`platform_id`	INTEGER NOT NULL,
	`description`	TEXT,
	`genres`	TEXT,
	`publisher`	TEXT,
	`developer`	TEXT,
	`release_date`	TEXT,
	`players`	TEXT,
	`rating`	TEXT,
    `extra_parameter`   TEXT,
    `favorite` INTEGER DEFAULT 0,
    `last_played` INTEGER DEFAULT 0
);
CREATE TABLE "mapping" (
    `id` TEXT NOT NULL,
    `left` INTEGER,
    `right` INTEGER,
    `up` INTEGER,
    `down` INTEGER,
    `a` INTEGER,
    `b` INTEGER,
    `start` INTEGER,
    `select` INTEGER,
    `l` INTEGER,
    `r` INTEGER
);
INSERT INTO `mehstation` VALUES ('schema','1');
