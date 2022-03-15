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
import org.apache.kafka.common.Uuid;
import org.apache.kafka.common.errors.UnsupportedVersionException;
import org.apache.kafka.common.protocol.ApiMessage;
import org.apache.kafka.common.protocol.Message;
import org.apache.kafka.common.protocol.MessageSizeAccumulator;
import org.apache.kafka.common.protocol.MessageUtil;
import org.apache.kafka.common.protocol.ObjectSerializationCache;
import org.apache.kafka.common.protocol.Readable;
import org.apache.kafka.common.protocol.Writable;
import org.apache.kafka.common.protocol.types.ArrayOf;
import org.apache.kafka.common.protocol.types.CompactArrayOf;
import org.apache.kafka.common.protocol.types.Field;
import org.apache.kafka.common.protocol.types.RawTaggedField;
import org.apache.kafka.common.protocol.types.RawTaggedFieldWriter;
import org.apache.kafka.common.protocol.types.Schema;
import org.apache.kafka.common.protocol.types.Type;
import org.apache.kafka.common.utils.ByteUtils;

import static org.apache.kafka.common.protocol.types.Field.TaggedFieldsSection;


public class MetadataRequestData implements ApiMessage {
    List<MetadataRequestTopic> topics;
    boolean allowAutoTopicCreation;
    boolean includeClusterAuthorizedOperations;
    boolean includeTopicAuthorizedOperations;
    private List<RawTaggedField> _unknownTaggedFields;
    
    public static final Schema SCHEMA_0 =
        new Schema(
            new Field("topics", new ArrayOf(MetadataRequestTopic.SCHEMA_0), "The topics to fetch metadata for.")
        );
    
    public static final Schema SCHEMA_1 =
        new Schema(
            new Field("topics", ArrayOf.nullable(MetadataRequestTopic.SCHEMA_0), "The topics to fetch metadata for.")
        );
    
    public static final Schema SCHEMA_2 = SCHEMA_1;
    
    public static final Schema SCHEMA_3 = SCHEMA_2;
    
    public static final Schema SCHEMA_4 =
        new Schema(
            new Field("topics", ArrayOf.nullable(MetadataRequestTopic.SCHEMA_0), "The topics to fetch metadata for."),
            new Field("allow_auto_topic_creation", Type.BOOLEAN, "If this is true, the broker may auto-create topics that we requested which do not already exist, if it is configured to do so.")
        );
    
    public static final Schema SCHEMA_5 = SCHEMA_4;
    
    public static final Schema SCHEMA_6 = SCHEMA_5;
    
    public static final Schema SCHEMA_7 = SCHEMA_6;
    
    public static final Schema SCHEMA_8 =
        new Schema(
            new Field("topics", ArrayOf.nullable(MetadataRequestTopic.SCHEMA_0), "The topics to fetch metadata for."),
            new Field("allow_auto_topic_creation", Type.BOOLEAN, "If this is true, the broker may auto-create topics that we requested which do not already exist, if it is configured to do so."),
            new Field("include_cluster_authorized_operations", Type.BOOLEAN, "Whether to include cluster authorized operations."),
            new Field("include_topic_authorized_operations", Type.BOOLEAN, "Whether to include topic authorized operations.")
        );
    
    public static final Schema SCHEMA_9 =
        new Schema(
            new Field("topics", CompactArrayOf.nullable(MetadataRequestTopic.SCHEMA_9), "The topics to fetch metadata for."),
            new Field("allow_auto_topic_creation", Type.BOOLEAN, "If this is true, the broker may auto-create topics that we requested which do not already exist, if it is configured to do so."),
            new Field("include_cluster_authorized_operations", Type.BOOLEAN, "Whether to include cluster authorized operations."),
            new Field("include_topic_authorized_operations", Type.BOOLEAN, "Whether to include topic authorized operations."),
            TaggedFieldsSection.of(
            )
        );
    
    public static final Schema SCHEMA_10 =
        new Schema(
            new Field("topics", CompactArrayOf.nullable(MetadataRequestTopic.SCHEMA_10), "The topics to fetch metadata for."),
            new Field("allow_auto_topic_creation", Type.BOOLEAN, "If this is true, the broker may auto-create topics that we requested which do not already exist, if it is configured to do so."),
            new Field("include_cluster_authorized_operations", Type.BOOLEAN, "Whether to include cluster authorized operations."),
            new Field("include_topic_authorized_operations", Type.BOOLEAN, "Whether to include topic authorized operations."),
            TaggedFieldsSection.of(
            )
        );
    
    public static final Schema SCHEMA_11 =
        new Schema(
            new Field("topics", CompactArrayOf.nullable(MetadataRequestTopic.SCHEMA_10), "The topics to fetch metadata for."),
            new Field("allow_auto_topic_creation", Type.BOOLEAN, "If this is true, the broker may auto-create topics that we requested which do not already exist, if it is configured to do so."),
            new Field("include_topic_authorized_operations", Type.BOOLEAN, "Whether to include topic authorized operations."),
            TaggedFieldsSection.of(
            )
        );
    
    public static final Schema SCHEMA_12 = SCHEMA_11;
    
    public static final Schema[] SCHEMAS = new Schema[] {
        SCHEMA_0,
        SCHEMA_1,
        SCHEMA_2,
        SCHEMA_3,
        SCHEMA_4,
        SCHEMA_5,
        SCHEMA_6,
        SCHEMA_7,
        SCHEMA_8,
        SCHEMA_9,
        SCHEMA_10,
        SCHEMA_11,
        SCHEMA_12
    };
    
    public static final short LOWEST_SUPPORTED_VERSION = 0;
    public static final short HIGHEST_SUPPORTED_VERSION = 12;
    
    public MetadataRequestData(Readable _readable, short _version) {
        read(_readable, _version);
    }
    
    public MetadataRequestData() {
        this.topics = new ArrayList<MetadataRequestTopic>(0);
        this.allowAutoTopicCreation = true;
        this.includeClusterAuthorizedOperations = false;
        this.includeTopicAuthorizedOperations = false;
    }
    
    @Override
    public short apiKey() {
        return 3;
    }
    
    @Override
    public short lowestSupportedVersion() {
        return 0;
    }
    
    @Override
    public short highestSupportedVersion() {
        return 12;
    }
    
    @Override
    public void read(Readable _readable, short _version) {
        {
            if (_version >= 9) {
                int arrayLength;
                arrayLength = _readable.readUnsignedVarint() - 1;
                if (arrayLength < 0) {
                    this.topics = null;
                } else {
                    ArrayList<MetadataRequestTopic> newCollection = new ArrayList<>(arrayLength);
                    for (int i = 0; i < arrayLength; i++) {
                        newCollection.add(new MetadataRequestTopic(_readable, _version));
                    }
                    this.topics = newCollection;
                }
            } else {
                int arrayLength;
                arrayLength = _readable.readInt();
                if (arrayLength < 0) {
                    if (_version >= 1) {
                        this.topics = null;
                    } else {
                        throw new RuntimeException("non-nullable field topics was serialized as null");
                    }
                } else {
                    ArrayList<MetadataRequestTopic> newCollection = new ArrayList<>(arrayLength);
                    for (int i = 0; i < arrayLength; i++) {
                        newCollection.add(new MetadataRequestTopic(_readable, _version));
                    }
                    this.topics = newCollection;
                }
            }
        }
        if (_version >= 4) {
            this.allowAutoTopicCreation = _readable.readByte() != 0;
        } else {
            this.allowAutoTopicCreation = true;
        }
        if ((_version >= 8) && (_version <= 10)) {
            this.includeClusterAuthorizedOperations = _readable.readByte() != 0;
        } else {
            this.includeClusterAuthorizedOperations = false;
        }
        if (_version >= 8) {
            this.includeTopicAuthorizedOperations = _readable.readByte() != 0;
        } else {
            this.includeTopicAuthorizedOperations = false;
        }
        this._unknownTaggedFields = null;
        if (_version >= 9) {
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
        if (_version >= 9) {
            if (topics == null) {
                _writable.writeUnsignedVarint(0);
            } else {
                _writable.writeUnsignedVarint(topics.size() + 1);
                for (MetadataRequestTopic topicsElement : topics) {
                    topicsElement.write(_writable, _cache, _version);
                }
            }
        } else {
            if (topics == null) {
                if (_version >= 1) {
                    _writable.writeInt(-1);
                } else {
                    throw new NullPointerException();
                }
            } else {
                _writable.writeInt(topics.size());
                for (MetadataRequestTopic topicsElement : topics) {
                    topicsElement.write(_writable, _cache, _version);
                }
            }
        }
        if (_version >= 4) {
            _writable.writeByte(allowAutoTopicCreation ? (byte) 1 : (byte) 0);
        } else {
            if (!this.allowAutoTopicCreation) {
                throw new UnsupportedVersionException("Attempted to write a non-default allowAutoTopicCreation at version " + _version);
            }
        }
        if ((_version >= 8) && (_version <= 10)) {
            _writable.writeByte(includeClusterAuthorizedOperations ? (byte) 1 : (byte) 0);
        } else {
            if (this.includeClusterAuthorizedOperations) {
                throw new UnsupportedVersionException("Attempted to write a non-default includeClusterAuthorizedOperations at version " + _version);
            }
        }
        if (_version >= 8) {
            _writable.writeByte(includeTopicAuthorizedOperations ? (byte) 1 : (byte) 0);
        } else {
            if (this.includeTopicAuthorizedOperations) {
                throw new UnsupportedVersionException("Attempted to write a non-default includeTopicAuthorizedOperations at version " + _version);
            }
        }
        RawTaggedFieldWriter _rawWriter = RawTaggedFieldWriter.forFields(_unknownTaggedFields);
        _numTaggedFields += _rawWriter.numFields();
        if (_version >= 9) {
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
        if (topics == null) {
            if (_version >= 9) {
                _size.addBytes(1);
            } else {
                _size.addBytes(4);
            }
        } else {
            if (_version >= 9) {
                _size.addBytes(ByteUtils.sizeOfUnsignedVarint(topics.size() + 1));
            } else {
                _size.addBytes(4);
            }
            for (MetadataRequestTopic topicsElement : topics) {
                topicsElement.addSize(_size, _cache, _version);
            }
        }
        if (_version >= 4) {
            _size.addBytes(1);
        }
        if ((_version >= 8) && (_version <= 10)) {
            _size.addBytes(1);
        }
        if (_version >= 8) {
            _size.addBytes(1);
        }
        if (_unknownTaggedFields != null) {
            _numTaggedFields += _unknownTaggedFields.size();
            for (RawTaggedField _field : _unknownTaggedFields) {
                _size.addBytes(ByteUtils.sizeOfUnsignedVarint(_field.tag()));
                _size.addBytes(ByteUtils.sizeOfUnsignedVarint(_field.size()));
                _size.addBytes(_field.size());
            }
        }
        if (_version >= 9) {
            _size.addBytes(ByteUtils.sizeOfUnsignedVarint(_numTaggedFields));
        } else {
            if (_numTaggedFields > 0) {
                throw new UnsupportedVersionException("Tagged fields were set, but version " + _version + " of this message does not support them.");
            }
        }
    }
    
    @Override
    public boolean equals(Object obj) {
        if (!(obj instanceof MetadataRequestData)) return false;
        MetadataRequestData other = (MetadataRequestData) obj;
        if (this.topics == null) {
            if (other.topics != null) return false;
        } else {
            if (!this.topics.equals(other.topics)) return false;
        }
        if (allowAutoTopicCreation != other.allowAutoTopicCreation) return false;
        if (includeClusterAuthorizedOperations != other.includeClusterAuthorizedOperations) return false;
        if (includeTopicAuthorizedOperations != other.includeTopicAuthorizedOperations) return false;
        return MessageUtil.compareRawTaggedFields(_unknownTaggedFields, other._unknownTaggedFields);
    }
    
    @Override
    public int hashCode() {
        int hashCode = 0;
        hashCode = 31 * hashCode + (topics == null ? 0 : topics.hashCode());
        hashCode = 31 * hashCode + (allowAutoTopicCreation ? 1231 : 1237);
        hashCode = 31 * hashCode + (includeClusterAuthorizedOperations ? 1231 : 1237);
        hashCode = 31 * hashCode + (includeTopicAuthorizedOperations ? 1231 : 1237);
        return hashCode;
    }
    
    @Override
    public MetadataRequestData duplicate() {
        MetadataRequestData _duplicate = new MetadataRequestData();
        if (topics == null) {
            _duplicate.topics = null;
        } else {
            ArrayList<MetadataRequestTopic> newTopics = new ArrayList<MetadataRequestTopic>(topics.size());
            for (MetadataRequestTopic _element : topics) {
                newTopics.add(_element.duplicate());
            }
            _duplicate.topics = newTopics;
        }
        _duplicate.allowAutoTopicCreation = allowAutoTopicCreation;
        _duplicate.includeClusterAuthorizedOperations = includeClusterAuthorizedOperations;
        _duplicate.includeTopicAuthorizedOperations = includeTopicAuthorizedOperations;
        return _duplicate;
    }
    
    @Override
    public String toString() {
        return "MetadataRequestData("
            + "topics=" + ((topics == null) ? "null" : MessageUtil.deepToString(topics.iterator()))
            + ", allowAutoTopicCreation=" + (allowAutoTopicCreation ? "true" : "false")
            + ", includeClusterAuthorizedOperations=" + (includeClusterAuthorizedOperations ? "true" : "false")
            + ", includeTopicAuthorizedOperations=" + (includeTopicAuthorizedOperations ? "true" : "false")
            + ")";
    }
    
    public List<MetadataRequestTopic> topics() {
        return this.topics;
    }
    
    public boolean allowAutoTopicCreation() {
        return this.allowAutoTopicCreation;
    }
    
    public boolean includeClusterAuthorizedOperations() {
        return this.includeClusterAuthorizedOperations;
    }
    
    public boolean includeTopicAuthorizedOperations() {
        return this.includeTopicAuthorizedOperations;
    }
    
    @Override
    public List<RawTaggedField> unknownTaggedFields() {
        if (_unknownTaggedFields == null) {
            _unknownTaggedFields = new ArrayList<>(0);
        }
        return _unknownTaggedFields;
    }
    
    public MetadataRequestData setTopics(List<MetadataRequestTopic> v) {
        this.topics = v;
        return this;
    }
    
    public MetadataRequestData setAllowAutoTopicCreation(boolean v) {
        this.allowAutoTopicCreation = v;
        return this;
    }
    
    public MetadataRequestData setIncludeClusterAuthorizedOperations(boolean v) {
        this.includeClusterAuthorizedOperations = v;
        return this;
    }
    
    public MetadataRequestData setIncludeTopicAuthorizedOperations(boolean v) {
        this.includeTopicAuthorizedOperations = v;
        return this;
    }
    
    public static class MetadataRequestTopic implements Message {
        Uuid topicId;
        String name;
        private List<RawTaggedField> _unknownTaggedFields;
        
        public static final Schema SCHEMA_0 =
            new Schema(
                new Field("name", Type.STRING, "The topic name.")
            );
        
        public static final Schema SCHEMA_1 = SCHEMA_0;
        
        public static final Schema SCHEMA_2 = SCHEMA_1;
        
        public static final Schema SCHEMA_3 = SCHEMA_2;
        
        public static final Schema SCHEMA_4 = SCHEMA_3;
        
        public static final Schema SCHEMA_5 = SCHEMA_4;
        
        public static final Schema SCHEMA_6 = SCHEMA_5;
        
        public static final Schema SCHEMA_7 = SCHEMA_6;
        
        public static final Schema SCHEMA_8 = SCHEMA_7;
        
        public static final Schema SCHEMA_9 =
            new Schema(
                new Field("name", Type.COMPACT_STRING, "The topic name."),
                TaggedFieldsSection.of(
                )
            );
        
        public static final Schema SCHEMA_10 =
            new Schema(
                new Field("topic_id", Type.UUID, "The topic id."),
                new Field("name", Type.COMPACT_NULLABLE_STRING, "The topic name."),
                TaggedFieldsSection.of(
                )
            );
        
        public static final Schema SCHEMA_11 = SCHEMA_10;
        
        public static final Schema SCHEMA_12 = SCHEMA_11;
        
        public static final Schema[] SCHEMAS = new Schema[] {
            SCHEMA_0,
            SCHEMA_1,
            SCHEMA_2,
            SCHEMA_3,
            SCHEMA_4,
            SCHEMA_5,
            SCHEMA_6,
            SCHEMA_7,
            SCHEMA_8,
            SCHEMA_9,
            SCHEMA_10,
            SCHEMA_11,
            SCHEMA_12
        };
        
        public static final short LOWEST_SUPPORTED_VERSION = 0;
        public static final short HIGHEST_SUPPORTED_VERSION = 12;
        
        public MetadataRequestTopic(Readable _readable, short _version) {
            read(_readable, _version);
        }
        
        public MetadataRequestTopic() {
            this.topicId = Uuid.ZERO_UUID;
            this.name = "";
        }
        
        
        @Override
        public short lowestSupportedVersion() {
            return 0;
        }
        
        @Override
        public short highestSupportedVersion() {
            return 12;
        }
        
        @Override
        public void read(Readable _readable, short _version) {
            if (_version > 12) {
                throw new UnsupportedVersionException("Can't read version " + _version + " of MetadataRequestTopic");
            }
            if (_version >= 10) {
                this.topicId = _readable.readUuid();
            } else {
                this.topicId = Uuid.ZERO_UUID;
            }
            {
                int length;
                if (_version >= 9) {
                    length = _readable.readUnsignedVarint() - 1;
                } else {
                    length = _readable.readShort();
                }
                if (length < 0) {
                    if (_version >= 10) {
                        this.name = null;
                    } else {
                        throw new RuntimeException("non-nullable field name was serialized as null");
                    }
                } else if (length > 0x7fff) {
                    throw new RuntimeException("string field name had invalid length " + length);
                } else {
                    this.name = _readable.readString(length);
                }
            }
            this._unknownTaggedFields = null;
            if (_version >= 9) {
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
            if (_version >= 10) {
                _writable.writeUuid(topicId);
            }
            if (name == null) {
                if (_version >= 10) {
                    _writable.writeUnsignedVarint(0);
                } else {
                    throw new NullPointerException();
                }
            } else {
                byte[] _stringBytes = _cache.getSerializedValue(name);
                if (_version >= 9) {
                    _writable.writeUnsignedVarint(_stringBytes.length + 1);
                } else {
                    _writable.writeShort((short) _stringBytes.length);
                }
                _writable.writeByteArray(_stringBytes);
            }
            RawTaggedFieldWriter _rawWriter = RawTaggedFieldWriter.forFields(_unknownTaggedFields);
            _numTaggedFields += _rawWriter.numFields();
            if (_version >= 9) {
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
            if (_version > 12) {
                throw new UnsupportedVersionException("Can't size version " + _version + " of MetadataRequestTopic");
            }
            if (_version >= 10) {
                _size.addBytes(16);
            }
            if (name == null) {
                if (_version >= 9) {
                    _size.addBytes(1);
                } else {
                    _size.addBytes(2);
                }
            } else {
                byte[] _stringBytes = name.getBytes(StandardCharsets.UTF_8);
                if (_stringBytes.length > 0x7fff) {
                    throw new RuntimeException("'name' field is too long to be serialized");
                }
                _cache.cacheSerializedValue(name, _stringBytes);
                if (_version >= 9) {
                    _size.addBytes(_stringBytes.length + ByteUtils.sizeOfUnsignedVarint(_stringBytes.length + 1));
                } else {
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
            if (_version >= 9) {
                _size.addBytes(ByteUtils.sizeOfUnsignedVarint(_numTaggedFields));
            } else {
                if (_numTaggedFields > 0) {
                    throw new UnsupportedVersionException("Tagged fields were set, but version " + _version + " of this message does not support them.");
                }
            }
        }
        
        @Override
        public boolean equals(Object obj) {
            if (!(obj instanceof MetadataRequestTopic)) return false;
            MetadataRequestTopic other = (MetadataRequestTopic) obj;
            if (!this.topicId.equals(other.topicId)) return false;
            if (this.name == null) {
                if (other.name != null) return false;
            } else {
                if (!this.name.equals(other.name)) return false;
            }
            return MessageUtil.compareRawTaggedFields(_unknownTaggedFields, other._unknownTaggedFields);
        }
        
        @Override
        public int hashCode() {
            int hashCode = 0;
            hashCode = 31 * hashCode + topicId.hashCode();
            hashCode = 31 * hashCode + (name == null ? 0 : name.hashCode());
            return hashCode;
        }
        
        @Override
        public MetadataRequestTopic duplicate() {
            MetadataRequestTopic _duplicate = new MetadataRequestTopic();
            _duplicate.topicId = topicId;
            if (name == null) {
                _duplicate.name = null;
            } else {
                _duplicate.name = name;
            }
            return _duplicate;
        }
        
        @Override
        public String toString() {
            return "MetadataRequestTopic("
                + "topicId=" + topicId.toString()
                + ", name=" + ((name == null) ? "null" : "'" + name.toString() + "'")
                + ")";
        }
        
        public Uuid topicId() {
            return this.topicId;
        }
        
        public String name() {
            return this.name;
        }
        
        @Override
        public List<RawTaggedField> unknownTaggedFields() {
            if (_unknownTaggedFields == null) {
                _unknownTaggedFields = new ArrayList<>(0);
            }
            return _unknownTaggedFields;
        }
        
        public MetadataRequestTopic setTopicId(Uuid v) {
            this.topicId = v;
            return this;
        }
        
        public MetadataRequestTopic setName(String v) {
            this.name = v;
            return this;
        }
    }
}
