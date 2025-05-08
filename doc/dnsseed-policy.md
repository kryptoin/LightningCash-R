# Expectations for DNS Seed Operators

As **LightningCash-R Core** is a growing project, DNS seeds are a crucial component for the early development and stability of the network. Given that this project is still in a rebuilding phase and there are only a few DNS seed hosts available, this policy seeks to provide basic guidelines for DNS seed operators, ensuring that they maintain security, reliability, and transparency.

Although **DNS seeds** are designed to minimize trust, they still introduce a small level of risk. Therefore, DNS seeds must be operated by entities or individuals who hold a certain level of trust within the **LightningCash-R** community.

Other implementations of **LightningCash-R** software may use the same seeds and may be more exposed. In light of this exposure, this document establishes the following basic expectations for DNS seed operators.

## DNS Seed Policy Guidelines

### 1. Trustworthy Operation

The DNS seed operator is expected to follow best security practices and maintain control over the DNS infrastructure. The operator should not sell or transfer control of the DNS seed to any third party. If any hosting services are used, they must also uphold these expectations.

### 2. Fair Selection of Nodes

The DNS seed operator must ensure that the DNS seed results consist solely of **fairly selected**, **functioning** **LightningCash-R** nodes from the public network. The selection process should be transparent and aim to avoid any biases in node selection.

### 3. Randomization of Results

DNS seed results should be randomized to avoid preferential treatment of any specific node or group. However, it is acceptable to make exceptions for urgent technical necessities. If such an exception occurs, it should be disclosed publicly.

### 4. DNS TTL (Time-to-Live)

The DNS seed should not serve DNS responses with a TTL of less than **one minute** to prevent clients from caching outdated seed information.

### 5. Logging of DNS Queries

Any logging of DNS queries should be limited to what is necessary for the operation of the service or for addressing the urgent health of the **LightningCash-R** network. Logs should not be retained longer than necessary, and operators must not disclose query logs to third parties.

### 6. Node Connectivity Data

Information obtained from the operator's node-spidering (i.e., discovery of nodes through network scanning) may be freely published or retained. However, the operator must ensure that such data was not influenced by biased node connectivity (e.g., by prioritizing or ignoring certain nodes, which violates expectation (2)).

### 7. Public Documentation

Operators are **encouraged**, but not required, to publicly document their operating practices. This ensures transparency and helps foster trust within the **LightningCash-R** community.

### 8. Contact Information

A **reachable email contact address** must be made publicly available for inquiries related to the operation of the DNS seed. This ensures that the community can easily reach out for support, questions, or concerns.

---

### Discontinuation of Service

If a DNS seed operator is unable to meet these expectations, they should cease operating the service and notify the active **LightningCash-R Core** development team. Additionally, they should post on the [LightningCash-R-dev forum](https://groups.google.com/forum/#!forum/LightningCash-R-dev) to inform the community about the cessation of service.

### Discussion of Exceptions

In some cases, behavior outside these expectations may be acceptable. However, if the operator deviates from these guidelines, they should **discuss** it publicly in advance to ensure transparency and maintain community trust.

---

_Last updated: May 2025_