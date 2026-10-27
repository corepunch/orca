(StackView Name="MetricUsers" class="card stack-card metric-card"
  (TextBlock Name="MetricUsersValue" class="metric-value"
    ForegroundColor=(bind "DataContext/PrimaryColor") Text=(bind "DataContext/Title"))
  (TextBlock Name="MetricUsersLabel" class="metric-label" Text=(bind "DataContext/Subtitle")))
