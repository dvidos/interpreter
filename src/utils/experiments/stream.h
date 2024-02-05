#ifndef _STREAM_H
#define _STREAM_H


typedef struct stream stream;



failable stream_write(stream *s, void *item);
failable_void stream_read(stream *s);
bool stream_finished(stream *s);

stream *new_stream(class *item_type);
stream *new_file_stream(class *item_type);
stream *new_memory_stream(class *item_type);
stream *new_stream_from_list()
...

#endif
