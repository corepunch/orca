(StackView Name="FeatureCard" class="card stack-card feature-card"
  (TextBlock Name="FeatureCardIcon" class="feature-card-icon" Text=(bind "DataContext/Icon")
    ForegroundColor=(bind "DataContext/PrimaryColor") BackgroundColor=(bind "DataContext/IconBackground"))
  (Node2D Name="FeatureCardSpacer" class="feature-card-spacer")
  (TextBlock Name="FeatureCardTitle" class="card-title" Text=(bind "DataContext/Title"))
  (TextBlock Name="FeatureCardBody" class="card-body" Text=(bind "DataContext/Body")))
