/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements. See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// THIS CODE IS AUTOMATICALLY GENERATED.  DO NOT EDIT.

package org.apache.kafka.common.message;

import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;
import org.apache.kafka.common.errors.UnsupportedVersionException;
import org.apache.kafka.common.protocol.ApiMessage;
import org.apache.kafka.common.protocol.MessageSizeAccumulator;
import org.apache.kafka.common.protocol.MessageUtil;
import org.apache.kafka.common.protocol.ObjectSerializationCache;
import org.apache.kafka.common.protocol.Readable;
import org.apache.kafka.common.protocol.Writable;
import org.apache.kafka.common.protocol.types.Field;
import org.apache.kafka.common.protocol.types.RawTaggedField;
import org.apache.kafka.common.protocol.types.RawTaggedFieldWriter;
import org.apache.kafka.common.protocol.types.Schema;
import org.apache.kafka.common.protocol.types.Type;
import org.apache.kafka.common.utils.ByteUtils;

import static org.apache.kafka.common.protocol.types.Field.TaggedFieldsSection;


public class RequestHeaderData implements ApiMessage {
    short requestApiKey;
    short requestApiVersion;
    int correlationId;
    String clientId;
    private List<RawTaggedField> _unknownTaggedFields;
    
    public static final Schema SCHEMA_0 =
        new Schema(
            new Field("request_api_key", Type.INT16, "The API key of this request."),
            new Field("request_api_version", Type.INT16, "The API version of this request."),
            new Field("correlation_id", Type.INT32, "The correlation ID of this request.")
        );
    
    public static final Schema SCHEMA_1 =
        new Schema(
            new Field("request_api_key", Type.INT16, "The API key of this request."),
            new Field("request_api_version", Type.INT16, "The API version of this request."),
            new Field("correlation_id", Type.INT32, "The correlation ID of this request."),
            new Field("client_id", Type.NULLABLE_STRING, "The client ID string.")
        );
    
    public static final Schema SCHEMA_2 =
        new Schema(
            new Field("request_api_key", Type.INT16, "The API key of this request."),
            new Field("request_api_version", Type.INT16, "The API version of this request."),
            new Field("correlation_id", Type.INT32, "The correlation ID of this request."),
            new Field("client_id", Type.NULLABLE_STRING, "The client ID string."),
            TaggedFieldsSection.of(
            )
        );
    
    public static final Schema[] SCHEMAS = new Schema[] {
        SCHEMA_0,
        SCHEMA_1,
        SCHEMA_2
    };
    
    public static final short LOWEST_SUPPORTED_VERSION = 0;
    public static final short HIGHEST_SUPPORTED_VERSION = 2;
    
    public RequestHeaderData(Readable _readable, short _version) {
        read(_readable, _version);
    }
    
    public RequestHeaderData() {
        this.requestApiKey = (short) 0;
        this.requestApiVersion = (short) 0;
        this.correlationId = 0;
        this.clientId = "";
    }
    
    @Override
    public short apiKey() {
        return -1;
    }
    
    @Override
    public short lowestSupportedVersion() {
        return 0;
    }
    
    @Override
    public short highestSupportedVersion() {
        return 2;
    }
    
    @Override
    public void read(Readable _readable, short _version) {
        this.requestApiKey = _readable.readShort();
        this.requestApiVersion = _readable.readShort();
        this.correlationId = _readable.readInt();
        if (_version >= 1) {
            int length;
            length = _readable.readShort();
            if (length < 0) {
                this.clientId = null;
            } else if (length > 0x7fff) {
                throw new RuntimeException("string field clientId had invalid length " + length);
            } else {
                this.clientId = _readable.readString(length);
            }
        } else {
            this.clientId = "";
        }
        this._unknownTaggedFields = null;
        if (_version >= 2) {
            int _numTaggedFields = _readable.readUnsignedVarint();
            for (int _i = 0; _i < _numTaggedFields; _i++) {
                int _tag = _readable.readUnsignedVarint();
                int _size = _readable.readUnsignedVarint();
                switch (_tag) {
                    default:
                        this._unknownTaggedFields = _readable.readUnknownTaggedField(this._unknownTaggedFields, _tag, _size);
                        break;
                }
            }
        }
    }
    
    @Override
    public void write(Writable _writable, ObjectSerializationCache _cache, short _version) {
        int _numTaggedFields = 0;
        _writable.writeShort(requestApiKey);
        _writable.writeShort(requestApiVersion);
        _writable.writeInt(correlationId);
        if (_version >= 1) {
            if (clientId == null) {
                _writable.writeShort((short) -1);
            } else {
                byte[] _stringBytes = _cache.getSerializedValue(clientId);
                _writable.writeShort((short) _stringBytes.length);
                _writable.writeByteArray(_stringBytes);
            }
        }
        RawTaggedFieldWriter _rawWriter = RawTaggedFieldWriter.forFields(_unknownTaggedFields);
        _numTaggedFields += _rawWriter.numFields();
        if (_version >= 2) {
            _writable.writeUnsignedVarint(_numTaggedFields);
            _rawWriter.writeRawTags(_writable, Integer.MAX_VALUE);
        } else {
            if (_numTaggedFields > 0) {
                throw new UnsupportedVersionException("Tagged fields were set, but version " + _version + " of this message does not support them.");
            }
        }
    }
    
    @Override
    public void addSize(MessageSizeAccumulator _size, ObjectSerializationCache _cache, short _version) {
        int _numTaggedFields = 0;
        _size.addBytes(2);
        _size.addBytes(2);
        _size.addBytes(4);
        if (_version >= 1) {
            if (clientId == null) {
                _size.addBytes(2);
            } else {
                byte[] _stringBytes = clientId.getBytes(StandardCharsets.UTF_8);
                if (_stringBytes.length > 0x7fff) {
                    throw new RuntimeException("'clientId' field is too long to be serialized");
                }
                _cache.cacheSerializedValue(clientId, _stringBytes);
                _size.addBytes(_stringBytes.length + 2);
            }
        }
        if (_unknownTaggedFields != null) {
            _numTaggedFields += _unknownTaggedFields.size();
            for (RawTaggedField _field : _unknownTaggedFields) {
                _size.addBytes(ByteUtils.sizeOfUnsignedVarint(_field.tag()));
                _size.addBytes(ByteUtils.sizeOfUnsignedVarint(_field.size()));
                _size.addBytes(_field.size());
            }
        }
        if (_version >= 2) {
            _size.addBytes(ByteUtils.sizeOfUnsignedVarint(_numTaggedFields));
        } else {
            if (_numTaggedFields > 0) {
                throw new UnsupportedVersionException("Tagged fields were set, but version " + _version + " of this message does not support them.");
            }
        }
    }
    
    @Override
    public boolean equals(Object obj) {
        if (!(obj instanceof RequestHeaderData)) return false;
        RequestHeaderData other = (RequestHeaderData) obj;
        if (requestApiKey != other.requestApiKey) return false;
        if (requestApiVersion != other.requestApiVersion) return false;
        if (correlationId != other.correlationId) return false;
        if (this.clientId == null) {
            if (other.clientId != null) return false;
        } else {
            if (!this.clientId.equals(other.clientId)) return false;
        }
        return MessageUtil.compareRawTaggedFields(_unknownTaggedFields, other._unknownTaggedFields);
    }
    
    @Override
    public int hashCode() {
        int hashCode = 0;
        hashCode = 31 * hashCode + requestApiKey;
        hashCode = 31 * hashCode + requestApiVersion;
        hashCode = 31 * hashCode + correlationId;
        hashCode = 31 * hashCode + (clientId == null ? 0 : clientId.hashCode());
        return hashCode;
    }
    
    @Override
    public RequestHeaderData duplicate() {
        RequestHeaderData _duplicate = new RequestHeaderData();
        _duplicate.requestApiKey = requestApiKey;
        _duplicate.requestApiVersion = requestApiVersion;
        _duplicate.correlationId = correlationId;
        if (clientId == null) {
            _duplicate.clientId = null;
        } else {
            _duplicate.clientId = clientId;
        }
        return _duplicate;
    }
    
    @Override
    public String toString() {
        return "RequestHeaderData("
            + "requestApiKey=" + requestApiKey
            + ", requestApiVersion=" + requestApiVersion
            + ", correlationId=" + correlationId
            + ", clientId=" + ((clientId == null) ? "null" : "'" + clientId.toString() + "'")
            + ")";
    }
    
    public short requestApiKey() {
        return this.requestApiKey;
    }
    
    public short requestApiVersion() {
        return this.requestApiVersion;
    }
    
    public int correlationId() {
        return this.correlationId;
    }
    
    public String clientId() {
        return this.clientId;
    }
    
    @Override
    public List<RawTaggedField> unknownTaggedFields() {
        if (_unknownTaggedFields == null) {
            _unknownTaggedFields = new ArrayList<>(0);
        }
        return _unknownTaggedFields;
    }
    
    public RequestHeaderData setRequestApiKey(short v) {
        this.requestApiKey = v;
        return this;
    }
    
    public RequestHeaderData setRequestApiVersion(short v) {
        this.requestApiVersion = v;
        return this;
    }
    
    public RequestHeaderData setCorrelationId(int v) {
        this.correlationId = v;
        return this;
    }
    
    public RequestHeaderData setClientId(String v) {
        this.clientId = v;
        return this;
    }
}
