# SISOP2-T1

## Dependency Tree

- shared_lib
  - shared_tests
  - server_lib
    - server
    - server_tests
  - client_lib
    - client
    - client_tests

CLion should show a _configuration_ for each of these targets, libs can only be compiled, but the others can run too. 

## Development

- Pay attention when adding files so that they are in the correct target, usually _client_lib_ or _server_lib_.

## Tests

Reference for the tests config and example.
https://www.jetbrains.com/help/clion/unit-testing-tutorial.html
