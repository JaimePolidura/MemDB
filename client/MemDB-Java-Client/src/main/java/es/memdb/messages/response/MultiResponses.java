package es.memdb.messages.response;

import es.memdb.utils.CountUpLatch;
import lombok.AllArgsConstructor;

import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;
import java.util.Optional;
import java.util.concurrent.TimeUnit;
import java.util.stream.Stream;

public final class MultiResponses {
    private final List<Response> responses = new ArrayList<>();
    private final CountUpLatch latch = new CountUpLatch();
    private final int maxNodes;

    public static MultiResponses fromCluster(int nNodes) {
        return new MultiResponses(nNodes);
    }

    public static MultiResponses fromSingleResponse(Response response) {
        MultiResponses multiResponses = new MultiResponses(1);
        multiResponses.responses.add(response);
        multiResponses.latch.countUp();

        return multiResponses;
    }

    public MultiResponses(int maxNodes) {
        this.maxNodes = maxNodes;
    }

    public Optional<MultiResponseResult> awaitQuorum(long time, TimeUnit timeUnit) {
        boolean isTimeout = latch.awaitMin(maxNodes / 2 + 1, time, timeUnit);

        return !isTimeout ?
                Optional.of(new MultiResponseResult(responses)) :
                Optional.empty();
    }

    @AllArgsConstructor
    public static class MultiResponseResult {
        private final List<Response> responses;

        public String anyResponse() {
            return this.responses.get(0).getResponse();
        }

        public String mostUptoDateResponse() {
            return responses.stream()
                    .max(Comparator.comparing(Response::getTimestamp))
                    .get()
                    .getResponse();
        }

        public boolean allSuccessful() {
            return responses.stream().allMatch(Response::isSuccessful);
        }

        public boolean anyResponseSuccessful() {
            return responses.stream().anyMatch(Response::isSuccessful);
        }

        public boolean anyResponseFailed() {
            return responses.stream().anyMatch(Response::isFailed);
        }

        public boolean allResponsesFailed() {
            return responses.stream().allMatch(Response::isFailed);
        }

        public List<String> allValues() {
            return responses.stream().map(Response::getResponse).toList();
        }

        public Stream<Response> stream() {
            return responses.stream();
        }
    }

    @AllArgsConstructor
    public static class MultipleResponseNotifier {
        private final MultiResponses multiResponses;

        public void addResponse(Response response) {
            synchronized (multiResponses) {
                multiResponses.responses.add(response);
                multiResponses.latch.countUp();
            }
        }
    }
}
