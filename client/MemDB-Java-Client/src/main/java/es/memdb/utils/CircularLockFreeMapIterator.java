package es.memdb.utils;

import lombok.AllArgsConstructor;

import java.util.Iterator;
import java.util.Map;
import java.util.concurrent.atomic.AtomicReference;

public final class CircularLockFreeMapIterator<K, V> implements Iterator<Map.Entry<K, V>> {
    private final Map<K, V> map;
    private AtomicReference<Iterator<K>> keyIterator;

    private AtomicReference<K> lastKey;

    public CircularLockFreeMapIterator(Map<K, V> map) {
        this.keyIterator = new AtomicReference<>(map.keySet().iterator());
        this.lastKey = new AtomicReference<>(this.keyIterator.get().next());
        this.map = map;
    }

    @Override
    public boolean hasNext() {
        return true;
    }

    @Override
    public Map.Entry<K, V> next() {
        while(true){
            Iterator<K> initialIterator = this.keyIterator.get();
            K initialValue = this.lastKey.get();

            if(!initialIterator.hasNext() && !this.keyIterator.compareAndSet(initialIterator, this.map.keySet().iterator())) {
                continue;
            }

            K nextKey = this.keyIterator.get().next();
            V nextValue = this.map.get(nextKey);

            if(this.lastKey.compareAndSet(initialValue, nextKey)){
                return new MapEntryImpl(nextKey, nextValue);
            }
        }
    }

    @AllArgsConstructor
    private class MapEntryImpl implements Map.Entry<K, V> {
        private final K key;
        private final V value;

        @Override
        public K getKey() {
            return this.key;
        }

        @Override
        public V getValue() {
            return this.value;
        }

        @Override
        public V setValue(V value) {
            throw new UnsupportedOperationException();
        }
    }
}
