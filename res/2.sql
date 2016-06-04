ALTER TABLE "platform" ADD COLUMN "discover_dir" TEXT;
ALTER TABLE "platform" ADD COLUMN "discover_ext" TEXT;
UPDATE "mehstation" SET "value" = 2 WHERE "name" = 'schema';
