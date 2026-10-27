(StackView Name="FeatureImageCard" class="card stack-card feature-image-card"
  (VectorView Name="FeatureImageCardIcon" class="feature-image-card-icon"
    Source=(bind "DataContext/Icon"))
  (TextBlock Name="FeatureImageCardTitle" class="card-title" Text=(bind "DataContext/Title"))
  (TextBlock Name="FeatureImageCardBody" class="card-body" Text=(bind "DataContext/Body")))
