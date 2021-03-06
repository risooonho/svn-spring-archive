ALTER TABLE archive RENAME to archive2;
CREATE VIEW archive AS
SELECT
  ar_namespace, ar_title, ar_text, ar_comment, ar_user, ar_user_text,
  ar_minor_edit, ar_flags, ar_rev_id, ar_text_id,
       TO_CHAR(ar_timestamp, 'YYYYMMDDHH24MISS') AS ar_timestamp
FROM archive2;

CREATE RULE archive_insert AS ON INSERT TO archive
DO INSTEAD INSERT INTO archive2 VALUES (
  NEW.ar_namespace, NEW.ar_title, NEW.ar_text, NEW.ar_comment, NEW.ar_user, NEW.ar_user_text,
  TO_DATE(NEW.ar_timestamp, 'YYYYMMDDHH24MISS'),
  NEW.ar_minor_edit, NEW.ar_flags, NEW.ar_rev_id, NEW.ar_text_id
);

