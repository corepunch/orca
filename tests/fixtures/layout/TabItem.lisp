; Test-only equivalent of Adventure/Prefabs/TabItem.xml.
(StackView Name="TabItem" class="tab-item"
  (VectorView Name="TabItemIcon" class="tab-icon" Source=(bind "DataContext/Icon") Stretch="None")
  (TextBlock Name="TabItemLabel" class="tab-label" Text=(bind "DataContext/Label")))
