(StackView Name="SignalCard" class="card signal-card"
  (VectorView Name="SignalCardIcon" class="signal-icon" Source=(bind "DataContext/Icon"))
  (StackView Name="SignalCardCopy" class="signal-copy"
    (TextBlock Name="SignalCardTitle" class="signal-title" Text=(bind "DataContext/Title"))
    (TextBlock Name="SignalCardBody" class="signal-body" Text=(bind "DataContext/Body"))))
