CREATE TABLE "platform"  (
	`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
	`name`	TEXT NOT NULL,
	`command`	TEXT NOT NULL,
    `icon`  TEXT
);
CREATE TABLE "mehstation" (
    `name` TEXT NOT NULL PRIMARY KEY UNIQUE,
    `value` TEXT
);
CREATE TABLE "executable_resource" (
	`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,
	`executable_id`	INTEGER NOT NULL,
	`type`	TEXT NOT NULL,
	`filepath`	TEXT NOT NULL
);
CREATE TABLE "executable" (
	`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,
	`display_name`	TEXT NOT NULL,
	`filepath`	TEXT NOT NULL,
	`platform_id`	INTEGER NOT NULL,
	`description`	TEXT,
	`genres`	TEXT,
	`publisher`	TEXT,
	`developer`	TEXT,
	`release_date`	TEXT,
	`players`	TEXT,
	`rating`	TEXT
);
INSERT INTO `mehstation` VALUES ('schema','1');
