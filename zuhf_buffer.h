#ifndef ZUHF_BUFFER_H
#define ZUHF_BUFFER_H

struct DATA_BUFFER{
	byte data[BUFFER_SIZE] = {};
	byte data_size = 0;
};

byte copy_to_buffer(DATA_BUFFER *buffer, const byte *data, byte bytes)
{
  if (((buffer->data_size) + bytes) < 255)
  {
    memcpy(buffer->data + buffer->data_size, data, bytes);
    (buffer->data_size) += bytes;
    return buffer->data_size;
  }else{
    if (Serial){
      Serial.println("[!] databuffer full! No additional data can be appended!");
    }
    return -1;
  }
}

#endif